if '%1'=='' (
echo The path to the solution root is required.
exit /b
)

if '%2'=='' (
echo The path to the project root is required.
exit /b
)

if '%3'=='' (
echo The path to the output directory is required.
exit /b
)

for /F "usebackq" %%i in (%2\resource_gen\prilist.txt) do echo %1\%%i > %2\resource_gen\pri.resfiles

makepri new /pr %2 /cf %2\resource_gen\priconfig.xml /of %3 /o