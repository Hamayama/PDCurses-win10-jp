pause starting.
cd %~dp0

set GAUCHE_PATH=gosh
set FILE1=gen_eaw_cdata.scm

%GAUCHE_PATH% %FILE1% 0
%GAUCHE_PATH% %FILE1% 1
%GAUCHE_PATH% %FILE1% 2
%GAUCHE_PATH% %FILE1% 3

pause finished.
