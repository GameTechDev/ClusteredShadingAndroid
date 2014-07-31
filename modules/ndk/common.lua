--
-- common.lua
-- Android exporter module for Premake - support code.
-- Copyright (c) 2014 Will Vale and the Premake project
--

local ndk       = premake.modules.ndk
local project   = premake.project

-- Constants
ndk.ANDROID     = 'android'
ndk.JNI         = 'jni'
ndk.MAKEFILE    = 'Android.mk'
ndk.APPMAKEFILE = 'Application.mk'
ndk.MANIFEST    = 'AndroidManifest.xml'
ndk.GLES31      = 'GLESv31'
ndk.GLES30      = 'GLESv3'
ndk.GLES20      = 'GLESv2'
ndk.GLES10      = 'GLESv1_CM'
ndk.JAVA        = '.java'

-- Need to put makefiles in subdirectories by project configuration
function ndk.getProjectPath(this, cfg)
	-- e.g. c:/root/myconfig/myproject
	return path.join(this.location)--, cfg.buildcfg, this.name)
end

-- Is the given project valid for NDK builds?
function ndk.isValidProject(prj)
	-- Console apps don't make sense
	if prj.kind == premake.CONSOLEAPP then
		return false
	end

	-- Otherwise valid if it contains a C or C++ file (under Visual Studio it's convenient to have non-compiling projects sometimes)
	for cfg in project.eachconfig(prj) do
		for _,f in ipairs(cfg.files) do
			if path.iscppfile(f) or path.iscfile(f) then
				return true
			end
		end
	end

	-- Otherwise invalid
	return false
end

-- Extract API level from framework
function ndk.getApiLevel(cfg)
	if cfg.framework then
		local version, count = cfg.framework:gsub('android%-', '')
		if count == 1 then
			return tonumber(version)
		end
	end

	-- Unknown API level
	return 1
end
