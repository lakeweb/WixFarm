rem "C:\Program Files (x86)\WiX Toolset v3.11\bin\candle.exe" /?
pause
"C:\Program Files (x86)\WiX Toolset v3.11\bin\candle.exe" -ext WixUIExtension -out ./bin/product.wixobj ./bin/product.wxs
rem  -out ./bin/product.wixobj
"C:\Program Files (x86)\WiX Toolset v3.11\bin\light.exe" ./bin/product.wixobj -out demo.msi -b C:\cpp\ReserveAnalyst_142\Program2\User
pause