cd .\

echo cd .\ > tmp_win.bat
echo call build_win32 clean >> tmp_win.bat
echo call build_win32 exe >> tmp_win.bat

echo cd .\ > tmp_vx.bat
echo call build_vxworks clean >> tmp_vx.bat
echo call build_vxworks exe >> tmp_vx.bat

start tmp_vx.bat
start tmp_win.bat

exit
