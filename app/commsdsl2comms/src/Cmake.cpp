#include "Cmake.h"

#include <fstream>

#include <boost/filesystem.hpp>

#include "Generator.h"
#include "common.h"

namespace bf = boost::filesystem;

namespace commsdsl2comms
{

bool Cmake::write(Generator& generator)
{
    Cmake obj(generator);
    return obj.writeMain() && obj.writePlugin();
}

bool Cmake::writeMain() const
{
    auto dir = m_generator.outputDir();
    if (dir.empty()) {
        return false;
    }

    bf::path filePath(dir);
    filePath /= common::cmakeListsFileStr();

    std::string filePathStr(filePath.string());

    m_generator.logger().info("Generating " + filePathStr);
    std::ofstream stream(filePathStr);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePathStr + "\" for writing.");
        return false;
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PROJ_NAME", m_generator.schemaName()));
    replacements.insert(std::make_pair("PROJ_NAMESPACE", m_generator.mainNamespace()));
    replacements.insert(std::make_pair("CC_TAG", m_generator.commsChampionTag()));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForFile(common::cmakeListsFileStr())));

    static const std::string Template = 
        "cmake_minimum_required (VERSION 3.1)\n"
        "project (\"#^#PROJ_NAME#$#\")\n\n"
        "option (OPT_LIB_ONLY \"Install only protocol library, no other libraries/plugings are built/installed.\" OFF)\n"
        "option (OPT_THIS_AND_COMMS_LIBS_ONLY \"Install this protocol and COMMS libraries only, no other applications/plugings are built/installed.\" OFF)\n"
        "option (OPT_FULL_SOLUTION \"Build and install full solution, including CommsChampion sources.\" ON)\n"
        "option (OPT_NO_WARN_AS_ERR \"Do NOT treat warning as error\" OFF)\n\n"
        "option (OPT_NO_CCACHE \"Disable use of ccache on UNIX system\" OFF)\n"
        "# Other parameters:\n"
        "# OPT_INSTALL_DIR - Custom install directory.\n"
        "# OPT_QT_DIR - Path to custom Qt5 install directory.\n"
        "# OPT_CC_MAIN_INSTALL_DIR - Path to CommsChampion install directory (if such already built).\n\n"
        "if (NOT CMAKE_CXX_STANDARD)\n"
        "    set (CMAKE_CXX_STANDARD 11)\n"
        "endif()\n\n"
        "set (INSTALL_DIR ${CMAKE_BINARY_DIR}/install)\n"
        "if (NOT \"${OPT_INSTALL_DIR}\" STREQUAL \"\")\n"
        "    set (INSTALL_DIR \"${OPT_INSTALL_DIR}\")\n"
        "endif ()\n\n"
        "include(GNUInstallDirs)\n"
        "set (LIB_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR})\n"
        "set (BIN_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_BINDIR})\n"
        "set (INC_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_INCLUDEDIR})\n"
        "set (CONFIG_INSTALL_DIR ${INSTALL_DIR}/config)\n"
        "set (PLUGIN_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/CommsChampion/plugin)\n"
        "set (DOC_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_DOCDIR})\n\n"
        "install (\n"
        "    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/#^#PROJ_NAMESPACE#$#\n"
        "    DESTINATION ${INC_INSTALL_DIR}\n"
        ")\n\n"
        "find_package (Doxygen)\n"
        "if (DOXYGEN_FOUND)\n"
        "    set (doc_output_dir \"${DOC_INSTALL_DIR}\")\n"
        "    make_directory (${doc_output_dir})\n\n"
        "    set (match_str \"OUTPUT_DIRECTORY[^\\n]*\")\n"
        "    set (replacement_str \"OUTPUT_DIRECTORY = ${doc_output_dir}\")\n"
        "    set (config_file \"${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen.conf\")\n"
        "    set (OPT_DOXYGEN_CONFIG_FILE \"${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf\")\n\n"
        "    file (READ ${config_file} config_text)\n"
        "    string (REGEX REPLACE \"${match_str}\" \"${replacement_str}\" modified_config_text \"${config_text}\")\n"
        "    file (WRITE \"${OPT_DOXYGEN_CONFIG_FILE}\" \"${modified_config_text}\")\n"
        "    set (doc_tgt \"doc_#^#PROJ_NAMESPACE#$#\")\n"
        "    add_custom_target (\"${doc_tgt}\"\n"
        "        COMMAND ${DOXYGEN_EXECUTABLE} ${OPT_DOXYGEN_CONFIG_FILE}\n"
        "        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})\n"
        "endif ()\n\n"
        "if (OPT_LIB_ONLY)\n"
        "    return ()\n"
        "endif ()\n\n"
        "######################################################################\n\n"
        "set (CC_EXTERNAL_TGT \"comms_champion_external\")\n"
        "include(ExternalProject)\n"
        "macro (externals install_dir build_cc)\n"
        "    set (cc_tag \"#^#CC_TAG#$#\")\n"
        "    set (cc_main_dir \"${CMAKE_BINARY_DIR}/comms_champion\")\n"
        "    set (cc_src_dir \"${cc_main_dir}/src\")\n"
        "    set (cc_bin_dir \"${cc_main_dir}/build\")\n\n"
        "    if (NOT \"${OPT_QT_DIR}\" STREQUAL \"\")\n"
        "        set (cc_qt_dir_opt -DCC_QT_DIR=${OPT_QT_DIR})\n"
        "    endif ()\n\n"
        "    if (${build_cc})\n"
        "        set (CC_PLUGIN_LIBRARIES \"comms_champion\")\n"
        "        set (CC_COMMS_CHAMPION_FOUND TRUE)\n"
        "        set (CC_PLUGIN_LIBRARY_DIRS ${LIB_INSTALL_DIR})\n"
        "    else ()\n"
        "        set (ct_lib_only_opt -DCC_COMMS_LIB_ONLY=ON)\n"
        "    endif ()\n\n"
        "    ExternalProject_Add(\n"
        "        \"${CC_EXTERNAL_TGT}\"\n"
        "        PREFIX \"${cc_bin_dir}\"\n"
        "        STAMP_DIR \"${cc_bin_dir}\"\n"
        "        GIT_REPOSITORY \"https://github.com/arobenko/comms_champion.git\"\n"
        "        GIT_TAG \"${cc_tag}\"\n"
        "        SOURCE_DIR \"${cc_src_dir}\"\n"
        "        CMAKE_ARGS\n"
        "            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCC_INSTALL_DIR=${install_dir}\n"
        "            -DCC_NO_UNIT_TESTS=ON -DCC_NO_WARN_AS_ERR=ON -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}\n"
        "            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}\n"
        "            ${cc_qt_dir_opt} ${ct_lib_only_opt}\n"
        "        BINARY_DIR \"${cc_bin_dir}\"\n"
        "    )\n\n"
        "    set (CC_EXTERNAL TRUE)\n"
        "    set (CC_COMMS_FOUND TRUE)\n"
        "    set (CC_CMAKE_DIR ${LIB_INSTALL_DIR}/cmake)\n\n"
        "    include_directories(\"${install_dir}/${CMAKE_INSTALL_INCLUDEDIR}\")\n"
        "    link_directories(\"${install_dir}/${CMAKE_INSTALL_LIBDIR}\")\n"
        "endmacro()\n\n"
        "######################################################################\n\n"
        "if (OPT_THIS_AND_COMMS_LIBS_ONLY)\n"
        "    externals(${INSTALL_DIR} FALSE)\n"
        "    return()\n"
        "endif ()\n\n"
        "while (TRUE)\n"
        "    if (OPT_FULL_SOLUTION)\n"
        "        externals(${INSTALL_DIR} TRUE)\n"
        "        break()\n"
        "    endif ()\n\n"
        "    list (APPEND CMAKE_PREFIX_PATH \"${INSTALL_DIR}\")\n"
        "    if (NOT \"${OPT_CC_MAIN_INSTALL_DIR}\" STREQUAL \"\")\n"
        "        list (APPEND CMAKE_PREFIX_PATH \"${OPT_CC_MAIN_INSTALL_DIR}\")\n"
        "    endif ()\n\n"
        "    find_package(CommsChampion QUIET NO_MODULE)\n\n"
        "    if (NOT CC_COMMS_FOUND)\n"
        "        set (externals_install \"${CMAKE_BINARY_DIR}/ext_install\")\n"
        "        set (build_cc FALSE)\n"
        "        if ((NOT OPT_LIB_ONLY) AND (NOT OPT_THIS_AND_COMMS_LIBS_ONLY))\n"
        "            set (build_cc TRUE)\n"
        "        endif ()\n\n"
        "        externals(${externals_install} ${build_cc})\n"
        "        break()\n"
        "    endif ()\n\n"
        "    find_package(CommsChampion NO_MODULE)\n"
        "    if (CC_COMMS_FOUND)\n"
        "        include_directories(${CC_INCLUDE_DIRS})\n"
        "    endif ()\n\n"
        "    if (CC_COMMS_CHAMPION_FOUND)\n"
        "        link_directories(${CC_PLUGIN_LIBRARY_DIRS})\n"
        "        file (RELATIVE_PATH rel_plugin_install_path \"${CC_ROOT_DIR}\" \"${CC_PLUGIN_DIR}\")\n"
        "        set (PLUGIN_INSTALL_DIR \"${INSTALL_DIR}/${rel_plugin_install_path}\")\n"
        "    endif ()\n\n"
        "    break()\n"
        "endwhile()\n\n"
        "if (NOT \"${OPT_QT_DIR}\" STREQUAL \"\")\n"
        "    list (APPEND CMAKE_PREFIX_PATH ${OPT_QT_DIR})\n"
        "endif ()\n\n"
        "find_package(Qt5Core)\n\n"
        "if ((CMAKE_COMPILER_IS_GNUCC) OR (\"${CMAKE_CXX_COMPILER_ID}\" STREQUAL \"Clang\"))\n"
        "    set (extra_flags_list\n"
        "        \"-Wall\" \"-Wextra\" \"-Wcast-align\" \"-Wcast-qual\" \"-Wctor-dtor-privacy\"\n"
        "        \"-Wmissing-include-dirs\"\n"
        "        \"-Woverloaded-virtual\" \"-Wredundant-decls\" \"-Wshadow\" \"-Wundef\" \"-Wunused\"\n"
        "        \"-Wno-unknown-pragmas\" \"-fdiagnostics-show-option\"\n"
        "    )\n\n"
        "    if (CMAKE_COMPILER_IS_GNUCC)\n"
        "        list (APPEND extra_flags_list\n"
        "            \"-Wnoexcept\" \"-Wlogical-op\" \"-Wstrict-null-sentinel\"\n"
        "        )\n"
        "    endif ()\n\n"
        "    if (\"${CMAKE_CXX_COMPILER_ID}\" STREQUAL \"Clang\")\n"
        "        list (APPEND extra_flags_list\n"
        "           \"-Wno-dangling-field\" \"-Wno-unused-command-line-argument\"\n"
        "           \"-ftemplate-depth=1024\")\n"
        "    endif ()\n\n"
        "    if (NOT OPT_NO_WARN_AS_ERR)\n"
        "        list (APPEND extra_flags_list \"-Werror\")\n"
        "    endif ()\n\n"
        "    string(REPLACE \";\" \" \" extra_flags \"${extra_flags_list}\")\n"
        "    set (CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} ${extra_flags}\")\n"
        "elseif (MSVC)\n"
        "    add_definitions( \"/wd4503\" \"/wd4309\" \"/wd4267\" \"-D_SCL_SECURE_NO_WARNINGS\" \"/bigobj\")\n"
        "    if (NOT CC_NO_WARN_AS_ERR)\n"
        "        add_definitions(\"/WX\")\n"
        "    endif ()\n"
        "endif ()\n\n"
        "include_directories(\n"
        "    BEFORE\n"
        "    ${CMAKE_SOURCE_DIR}\n"
        "    ${CMAKE_SOURCE_DIR}/include\n"
        ")\n\n"
        "if ((UNIX) AND (NOT OPT_NO_CCACHE))\n"
        "    find_program(CCACHE_FOUND ccache)\n"
        "    if(CCACHE_FOUND)\n"
        "        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)\n"
        "        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)\n"
        "    endif()\n"
        "endif ()\n\n"
        "add_subdirectory(cc_plugin)\n\n"
        "#^#APPEND#$#\n";

    auto str = common::processTemplate(Template, replacements);
    stream << str;

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePathStr + "\".");
        return false;
    }
    return true;
}

bool Cmake::writePlugin() const
{
    auto dir = m_generator.pluginDir();
    if (dir.empty()) {
        return false;
    }

    bf::path filePath(dir);
    filePath /= common::cmakeListsFileStr();

    std::string filePathStr(filePath.string());

    m_generator.logger().info("Generating " + filePathStr);
    std::ofstream stream(filePathStr);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePathStr + "\" for writing.");
        return false;
    }

    common::StringsList calls;
    auto plugins = m_generator.getPlugins();
    for (auto* p : plugins) {
        auto pName = common::nameToClassCopy(p->adjustedName());
        calls.push_back("cc_plugin (\"" + pName + "\")");
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("SOURCES", m_generator.pluginCommonSources()));
    replacements.insert(std::make_pair("PLUGINS", common::listToString(calls, "\n", "\n")));

    std::vector<std::string> appendPath = {
        common::pluginNsStr(),
        common::cmakeListsFileStr()
    };
    
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForFile(appendPath)));

    static const std::string Template =
        "set (ALL_MESSAGES_LIB \"all_messages\")\n\n"
        "######################################################################\n\n"
        "function (cc_plugin_all_messages)\n"
        "    set (name \"${ALL_MESSAGES_LIB}\")\n"
        "    set (src\n"
        "        #^#SOURCES#$#\n"
        "    )\n\n"
        "    add_library (${name} STATIC ${src})\n"
        "    target_link_libraries (${name} ${CC_PLUGIN_LIBRARIES})\n"
        "    qt5_use_modules(${name} Core)\n"
        "    if (OPT_FULL_SOLUTION)\n"
        "        add_dependencies(${name} ${CC_EXTERNAL_TGT})\n"
        "    endif ()\n"
        "endfunction()\n\n"
        "######################################################################\n\n"
        "function (cc_plugin protocol)\n"
        "    set (name \"cc_plugin_${protocol}\")\n\n"
        "    set (meta_file \"${CMAKE_CURRENT_SOURCE_DIR}/${protocol}.json\")\n"
        "    set (stamp_file \"${CMAKE_CURRENT_BINARY_DIR}/${protocol}_refresh_stamp.txt\")\n\n"
        "    if ((NOT EXISTS ${stamp_file}) OR (${meta_file} IS_NEWER_THAN ${stamp_file}))\n"
        "        execute_process(\n"
        "            COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_SOURCE_DIR}/plugin/${protocol}Plugin.h)\n\n"
        "        execute_process(\n"
        "            COMMAND ${CMAKE_COMMAND} -E touch ${stamp_file})\n"
        "    endif ()\n\n"
        "    set (src\n"
        "        plugin/${protocol}Protocol.cpp\n"
        "        plugin/${protocol}Plugin.cpp\n"
        "    )\n\n"
        "    set (hdr\n"
        "        plugin/${protocol}Plugin.h\n"
        "    )\n\n"
        "    qt5_wrap_cpp(moc ${hdr})\n\n"
        "    set(extra_link_opts)\n"
        "    if (CMAKE_COMPILER_IS_GNUCC)\n"
        "        set(extra_link_opts \"-Wl,--no-undefined\")\n"
        "    endif ()\n\n"
        "    add_library (${name} MODULE ${src} ${moc})\n"
        "    target_link_libraries (${name} ${ALL_MESSAGES_LIB} ${CC_PLUGIN_LIBRARIES} ${extra_link_opts})\n"
        "    if (CMAKE_COMPILER_IS_GNUCC)\n"
        "        target_compile_options(${name} PRIVATE \"-ftemplate-backtrace-limit=0\")\n"
        "    endif ()\n\n"
        "    qt5_use_modules (${name} Core)\n"
        "    install (\n"
        "        TARGETS ${name}\n"
        "        DESTINATION ${PLUGIN_INSTALL_DIR})\n\n"
        "endfunction()\n\n"
        "######################################################################\n\n"
        "if (NOT Qt5Core_FOUND)\n"
        "    message (WARNING \"Can NOT compile protocol plugin due to missing QT5 Core library\")\n"
        "    return ()\n"
        "endif ()\n\n"
        "if (CMAKE_COMPILER_IS_GNUCC)\n"
        "    set (CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0\")\n"
        "endif ()\n\n"
        "cc_plugin_all_messages()\n\n"
        "#^#PLUGINS#$#\n"
        "#^#APPEND#$#\n";

    auto str = common::processTemplate(Template, replacements);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePathStr + "\".");
        return false;
    }
    return true;
}

} // namespace commsdsl2comms
