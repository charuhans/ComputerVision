set inputName=%1
set outputName=%2
@echo %inputName%

REM =========================== Clean ====================================
if exist _tmp_res_ del _tmp_res_ md\*.* /q
if not exist _tmp_res_ md _tmp_res_


REM =========================== Blur =====================================
@echo ..\..\%inputName% >PassportPhoto\Blur\blurInput.txt
if exist PassportPhoto\Blur\blurOutput.txt del PassportPhoto\Blur\blurOutput.txt /q
call matlab -nodesktop -nosplash -nojvm -sd "PassportPhoto\Blur" -r haar
call PassportPhoto\Release\wait.exe PassportPhoto\Blur\blurOutput.txt
move PassportPhoto\Blur\blurOutput.txt _tmp_res_\blur.txt


REM =========================== Exposure =================================
call PassportPhoto\Release\Exposure.exe %inputName% _tmp_res_\exposure.txt


REM =========================== Orientation ==============================
call PassportPhoto\Release\Orientation.exe haarcascades %inputName% _tmp_res_\orientation.txt


REM =========================== Eyes =====================================
call PassportPhoto\Release\EyeClose.exe %inputName% _tmp_res_\eyes.txt


REM =========================== Mouth ====================================
call PassportPhoto\Release\MouthClose.exe haarcascades %inputName% _tmp_res_\mouth.txt


REM =========================== Combine ==================================
@echo %inputName% >%outputName%
type _tmp_res_\blur.txt >>%outputName%
type _tmp_res_\exposure.txt >>%outputName%
type _tmp_res_\orientation.txt >>%outputName%
type _tmp_res_\eyes.txt >>%outputName%
type _tmp_res_\mouth.txt >>%outputName%
