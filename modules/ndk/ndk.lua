--
-- ndk.lua
-- Android exporter module for Premake.
-- Copyright (c) 2014 Will Vale and the Premake project
--

-- Define module
premake.modules.ndk = {}
local ndk       = premake.modules.ndk

-- Shortcuts
local api       = premake.api
local config    = premake.config
local make      = premake.make
local project   = premake.project
local solution  = premake.solution

--
-- Register Android configuration options with Premake.
--

-- Specify android ABIs
api.register {
	name = "abis",
	scope = "config",
	kind = "string",
	list = "true",
	allowed = {
		"all",
		"armeabi",
		"armeabi-v7a",
		"mips",
		"x86"
	}
}

-- Specify android STL support
api.register {
	name = "stl",
	scope = "config",
	kind = "string",
	allowed = {
		"gabi++_static",
		"gabi++_shared",
		"gnustl_static",
		"gnustl_shared",
		"stlport_static",
		"stlport_shared",
		"system"
	}
}

-- Specify android package name
api.register {
	name = "packagename",
	scope = "project",
	kind = "string"
}

-- Specify android package version
api.register {
	name = "packageversion",
	scope = "project",
	kind = "integer"
}

-- Specify android activity name
api.register {
	name = "activity",
	scope = "project",
	kind = "string"
}

-- Specify android activity base class
api.register {
	name = "baseactivity",
	scope = "project",
	kind = "string"
}

-- Specify android activity base package
api.register {
	name = "basepackagename",
	scope = "project",
	kind = "string"
}

-- Specify applicaton permissions
api.register {
	name = "permissions",
	scope = "config",
	kind = "string",
	list = true
}

-- Specify static libraries to link against
api.register {
	name  = "staticlibs",
	scope = "config",
	kind  = "string",
	list  = true
}

api.register {
	name  = "import",
	scope = "config",
	kind  = "string",
	list  = true
}
	
-- Retrieve module's path and allow relative includes 
local this_file = debug.getinfo(1, "S").source:sub(2) 
package.path = path.getdirectory(this_file) .. "/?.lua;".. package.path

-- Support code
require 'common'

-- Actions
require 'makefile'
require 'manifest'

return ndk