
project := "romulus_project".

project_version := "1.0.4".

buildfile_version := "1.0".

url_to_src := "https://github.com/JHG777000/romulus/archive/master.zip".

build romulus_build.

 options.

  on test_enable("-t", "--test", "Enable romulus test.").

  on toolchain_select("-s", "--select_toolchain=tool", "Select toolchain.").

 end options.

 get test_enable.

 get toolchain_select.

 if ( toolchain_select == nil && is_mac ).

  var toolchain_select := "clang".

 end if.

 if ( toolchain_select == nil && !is_mac ).

  var toolchain_select := "gcc".

 end if.

 url URLForIDK("https://raw.githubusercontent.com/JHG777000/IDK/master/buildfile").

 subproject IDKProject("local",URLForIDK,nil).

 return_output IDKProject.

 grab RKLibProject from IDKProject.

 grab GLFWProject from IDKProject.

 make filepath include_path from "resources" to "include".

 make filepath idk_include_path from "resources" to "include" from IDKProject.

 make filepath glfw_include_path from "resources" to "include" from GLFWProject.

 make filepath rklib_include_path from "resources" to "include" from RKLibProject.

 files romulus_files("src.directory").

 sources romulus_source(romulus_files).

 compiler romulus_compiler_flags("-Wall", "-I " + include_path, "-I " + rklib_include_path,
          "-I " + glfw_include_path, "-I " + idk_include_path).

 toolchain romulus_tool_chain(toolchain_select,romulus_compiler_flags).

 output romulus("library",romulus_source,romulus_tool_chain).

 if ( test_enable ).

  message("Running romulus_test...").

  grab IDK from IDKProject.

  grab RKLib from RKLibProject.

  grab libglfw3 from GLFWProject.

  files romulus_test_files("main.c").

  sources romulus_test_source(romulus_test_files,RKLib,libglfw3,IDK,romulus).

  output romulus_test("application",romulus_test_source,romulus_tool_chain).

  make filepath romulus_test_path from "build" to "romulus_test_output/romulus_test".

  make filepath png_path from "resources" to "test.png".

  run(romulus_test_path + " " + png_path).

  message("Ran romulus_test.").

 end if.

end build.

build clean_build.

 url URLForIDK("https://raw.githubusercontent.com/JHG777000/IDK/master/buildfile").

 subproject IDKProject("local",URLForIDK,"-b clean_build").

 message("Cleaning romulus...").

 clean("build").

end build.

default romulus_build.
