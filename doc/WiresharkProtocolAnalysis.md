# Wireshark Protocol Analysis
The [Wireshark](https://www.wireshark.org/) contains infrastructure for
providing custom protocol dissectors implemented in [lua](https://wiki.wireshark.org/lua).
The **commsdsl2wireshark** code generator allows generation of such code from the
[CommsDSL](https://commschamp.github.io/commsdsl_spec/) schema files.

## Produced Output
The output of the **commsdsl2wireshark** is just a couple of files.
One is the actual protocol implementation, named `<schema_name>.lua`.
At the end the file returns a protocol object. This file is expected to be
included by the actual "plugin" code which sets up all the wireshark network related configuration.
Another file is called `<schema_name>_plug.lua` which does exactly that.

## Protocol Analysis
To be able to use custom dissector there is a need to use `-X lua_script:/path/to/<schema>_plug.lua` command line parameters.
```
$> wireshark -X lua_script:/path/to/my_prot_plug.lua ...
```
When using `tshark` instead of `wireshark` make sure to pass protocol name using `-O` command line option:
```
$> wireshark -X lua_script:/path/to/my_prot_plug.lua -O "my_prot" ...
```
The protocol name is the first parameter in the `Proto(...)` invocation at the beginning of the generated `<schema>.lua` file.
```lua
local my_prot = Proto("my_prot", ...)
```

## Configuring Port and Transport
By default the generated `<schema>_plug.lua` will use port specified by `--default-port` command line option (or `12345` if not) as well as
allow dissection for both `TCP` and `UDP` transport. To change the default port use `-o "<schema>.port:<new_port>"` command line option
when invoking `wireshark` or `tshark`.
```
$> wireshark -X lua_script:/path/to/my_prot_plug.lua -o "my_prot.port:9999" ...
```
The transport selection values are defined in the generated `<schema>_plug.lua` file:
```lua
local my_prot_transport_type = {
    TCP = 0,
    UDP = 1,
    BOTH = 2
}
```
As was mentioned earlier the default are both TCP and UDP:
```lua
local my_prot_transport = my_prot_transport_type.BOTH
```
To be able to change it during `wireshark` or `tshark` invocation use `-o "<schema>.transport:<new_transport>"` command line option. Note that the value is numeric.
For example to allow only UDP port dissection use:
```
$> wireshark -X lua_script:/path/to/my_prot_plug.lua -o "my_prot.transport:1" ...
```
