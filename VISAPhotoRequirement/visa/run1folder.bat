set folderName=%1
set outputName=%2

@echo FileName	Blur	Expos	Orient	Eyes	Mouth	Overall >%outputName%
@echo --------------------------------------------------------- >>%outputName%
@echo 0=Failed    1=Passed    N/A=Cannot detec face/eyes/mouth >>%outputName%
@echo --------------------------------------------------------- >>%outputName%

for /f %%a in ('dir %folderName% /b') do (
	call run1file.bat %folderName%\%%a _tmp_res_\all.txt
	call PassportPhoto\Release\combine.exe _tmp_res_\all.txt >>%outputName%
)
