set QB=C:\Qt\6.5.0\msvc2019_64
set QDIR=%QB%\bin
set QDLL=.dll
set DST=.\RTE
mkdir %DST%
copy %QDIR%\Qt6Widgets%QDLL% %DST%
copy %QDIR%\Qt6Gui%QDLL% %DST%
copy %QDIR%\Qt6Core%QDLL% %DST%
copy %QDIR%\Qt6SerialPort%QDLL% %DST%

mkdir %DST%\platforms
copy %QB%\plugins\platforms\qwindows%QDLL% %DST%\platforms\qwindows%QDLL%
copy ..\build-TurbineTest-Desktop_Qt_6_5_0_MSVC2019_64bit-Release\release\TurbineTest.exe %DST%
copy ..\berchtold\qc\build-TurbineTest_druck-Desktop_Qt_6_5_0_MSVC2019_64bit-Release\release\TurbineTest_druck.exe %DST%

pause



