./gradlew tasks
------------------------------------------------------------
Tasks runnable from root project 'MyApp'
------------------------------------------------------------

Android tasks
-------------
androidDependencies - Displays the Android dependencies of the project.
signingReport - Displays the signing info for the base and test modules
sourceSets - Prints out all the source sets defined in this project.

Build tasks
-----------
assemble - Assemble main outputs for all the variants.
assembleAndroidTest - Assembles all the Test applications.
build - Assembles and tests this project.
buildDependents - Assembles and tests this project and all projects that depend on it.
buildNeeded - Assembles and tests this project and all projects it depends on.
bundle - Assemble bundles for all the variants.
clean - Deletes the build directory.
compileDebugAndroidTestSources
compileDebugSources
compileDebugUnitTestSources
compileReleaseSources
compileReleaseUnitTestSources

Build Setup tasks
-----------------
init - Initializes a new Gradle build.
wrapper - Generates Gradle wrapper files.

Help tasks
----------
buildEnvironment - Displays all buildscript dependencies declared in root project 'MyApp'.
dependencies - Displays all dependencies declared in root project 'MyApp'.
dependencyInsight - Displays the insight into a specific dependency in root project 'MyApp'.
help - Displays a help message.
javaToolchains - Displays the detected java toolchains.
outgoingVariants - Displays the outgoing variants of root project 'MyApp'.
projects - Displays the sub-projects of root project 'MyApp'.
properties - Displays the properties of root project 'MyApp'.
resolvableConfigurations - Displays the configurations that can be resolved in root project 'MyApp'.
tasks - Displays the tasks runnable from root project 'MyApp' (some of the displayed tasks may belong to subprojects).

Install tasks
-------------
installDebug - Installs the Debug build.
installDebugAndroidTest - Installs the android (on device) tests for the Debug build.
uninstallAll - Uninstall all applications.
uninstallDebug - Uninstalls the Debug build.
uninstallDebugAndroidTest - Uninstalls the android (on device) tests for the Debug build.
uninstallRelease - Uninstalls the Release build.

Verification tasks
------------------
check - Runs all checks.
checkJetifier - Checks whether Jetifier is needed for the current project
connectedAndroidTest - Installs and runs instrumentation tests for all flavors on connected devices.
connectedCheck - Runs all device checks on currently connected devices.
connectedDebugAndroidTest - Installs and runs the tests for debug on connected devices.
deviceAndroidTest - Installs and runs instrumentation tests using all Device Providers.
deviceCheck - Runs all device checks using Device Providers and Test Servers.
lint - Runs lint on the default variant.
lintAnalyzeDebug - Run lint analysis on the debug variant
lintAnalyzeRelease - Run lint analysis on the release variant
lintDebug - Print text output from the corresponding lint report task
lintFix - Runs lint on the default variant and applies any safe suggestions to the source code.
lintFixDebug - Fix lint on the debug variant
lintFixRelease - Fix lint on the release variant
lintRelease - Print text output from the corresponding lint report task
lintReportDebug - Run lint on the debug variant
lintReportRelease - Run lint on the release variant
lintVitalAnalyzeRelease - Run lint analysis with only the fatal issues enabled on the release variant
lintVitalRelease - Print text output from the corresponding lint report task
lintVitalReportRelease - Run lint with only the fatal issues enabled on the release variant
test - Run unit tests for all variants.
testDebugUnitTest - Run unit tests for the debug build.
testReleaseUnitTest - Run unit tests for the release build.
updateLintBaseline - Updates the lint baseline using the default variant.
updateLintBaselineDebug - Update the lint baseline using the debug variant
updateLintBaselineRelease - Update the lint baseline using the release variant

To see all tasks and more detail, run gradlew tasks --all

To see more detail about a task, run gradlew help --task <task>
