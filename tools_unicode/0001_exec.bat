pause starting.
cd %~dp0

set FILE1=gen_eaw_cdata.scm

gosh %FILE1% 0
gosh %FILE1% 1
gosh %FILE1% 2
gosh %FILE1% 3
gosh %FILE1% 4

pause finished.
