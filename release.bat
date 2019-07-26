"C:\Program Files (x86)\WiX Toolset v3.11\bin\candle.exe" ../wix/product.wxs -ext WixUIExtension
rem "C:\Program Files (x86)\WiX Toolset v3.11\bin\light.exe" -out demo.msi -b C:\cpp\ReserveAnalyst_142\Program2\User ../wix/product.wxs


"C:\Program Files (x86)\WiX Toolset v3.11\bin\light.exe" Product.wixobj -out ReserveAnalyst.msi
 
rem light.exe -out demo.msi -b product.wixobj fragment.wixobj -ext WixUIExtension
pause