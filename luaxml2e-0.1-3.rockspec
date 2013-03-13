
package = "luaxml2e"
version = "0.1-3"

source = {
    url = "https://github.com/andrepiske/luaxml2e/"
}

description = {
    summary = "libxml2 XML tree for lua",
    detailed = [[
        A binding of the libxml2 tree for lua.
    ]] ,
    homepage = "",
    license = "MIT",
}

dependencies = {
    "lua == 5.1",
}

build = {
    type = "builtin",

    modules = {
        luaxml2e = {
            sources = { "src/luaxml2e.c" },
            libraries = { "xml2", "lua5.1", "m" },
            incdirs = { "/usr/include/libxml2" },
        },
        luaxml2es = "src/luaxml2es.lua",
    },

}

