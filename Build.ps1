param(
    [switch]$Deploy,
    [ValidatePattern('^[A-Za-z0-9_-]+$')]
    [string]$DeployDirectoryName = 'KinSealStudioLite-Stage2-0.9.0' #20261110-7 change: Publish independently while the previous editor remains open.
)

# 20261107-6 add: Build the Qt editor and optionally create a self-contained deployment directory.
$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$QtRoot = 'G:\01_Professional\QT6\Installed\6.12.0\mingw_64'
$ToolRoot = 'G:\01_Professional\QT6\Installed\Tools'
$CMake = Join-Path $ToolRoot 'CMake_64\bin\cmake.exe'
$Ninja = Join-Path $ToolRoot 'Ninja\ninja.exe'
$Compiler = Join-Path $ToolRoot 'mingw1310_64\bin\g++.exe'
$CompilerBin = Join-Path $ToolRoot 'mingw1310_64\bin'
$env:Path = "$CompilerBin;$($QtRoot)\bin;$($ToolRoot)\Ninja;$env:Path"
$env:QT_QPA_PLATFORM = 'offscreen' # 20261108-7 change: Keep tests off the user's desktop.

& $CMake -S $ProjectRoot -B (Join-Path $ProjectRoot 'build') -G Ninja `
    "-DCMAKE_MAKE_PROGRAM=$Ninja" `
    "-DCMAKE_CXX_COMPILER=$Compiler" `
    "-DCMAKE_PREFIX_PATH=$QtRoot" `
    -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

& $CMake --build (Join-Path $ProjectRoot 'build') --parallel 4
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$Application = Join-Path $ProjectRoot 'build\KinSealStudioLite.exe'
$PayloadTest = Start-Process -FilePath $Application -ArgumentList '--verify-independent-payload' `
    -Wait -PassThru -WindowStyle Hidden # 20261108-10 add: Validate every embedded TEST segment without touching USB.
if ($PayloadTest.ExitCode -ne 0) {
    throw "The independent TEST payload verification failed with code $($PayloadTest.ExitCode)."
}
$SelfTest = Start-Process -FilePath $Application -ArgumentList '--self-test' `
    -Wait -PassThru -WindowStyle Hidden
if ($SelfTest.ExitCode -ne 0) {
    throw "The project round-trip self-test failed with code $($SelfTest.ExitCode)."
}
$StaticTextTest = Start-Process -FilePath (Join-Path $ProjectRoot 'build\KinSealStaticTextTests.exe') `
    -ArgumentList @('-o', ((Join-Path $ProjectRoot 'build\static-text-tests.txt') + ',txt')) `
    -Wait -PassThru -WindowStyle Hidden # 20261108-14 add: Qt event tests use the offscreen platform.
if ($StaticTextTest.ExitCode -ne 0) {
    Get-Content -LiteralPath (Join-Path $ProjectRoot 'build\static-text-tests.txt')
    throw "Static-text interaction tests failed: $($StaticTextTest.ExitCode)."
}
#20261109-0 add: Check native static/dynamic command ordering without accessing USB.
$DeviceTransferTest = Start-Process -FilePath (Join-Path $ProjectRoot 'build\KinSealDeviceTransferTests.exe') `
    -ArgumentList @('-o', ((Join-Path $ProjectRoot 'build\device-transfer-tests.txt') + ',txt')) `
    -Wait -PassThru -WindowStyle Hidden
if ($DeviceTransferTest.ExitCode -ne 0) {
    Get-Content -LiteralPath (Join-Path $ProjectRoot 'build\device-transfer-tests.txt')
    throw "Device transfer plan tests failed: $($DeviceTransferTest.ExitCode)."
}
#20261109-0 add: Record codec and macro compiler tests separately; no test opens USB.
foreach ($CodecTestName in @('KinSealProjectModelTests', 'KinSealStage1BackendTests', 'KinSealStage2DialogsTests', 'KinSealStage2GlobalToolsTests', 'KinSealStage2WorkflowTests', 'KinSealNativeControlTests', 'KinSealNativeWordControlTests', 'KinSealNativeStaticControlTests', 'KinSealBitButtonRendererTests', 'KinSealDeviceMacroTests', 'KinSealPlcAddressTests', 'KinSealPlcNativeCatalogTests', 'KinSealDeviceScanTests', 'KinSealNativeSourceTests', 'KinSealWordButtonDialogTests', 'KinSealLampDialogTests', 'KinSealNativeLampControlTests', 'KinSealFunctionButtonDialogTests', 'KinSealNativeFunctionControlTests')) { #20261110-5 change
    $CodecTestLog = Join-Path $ProjectRoot ("build\" + $CodecTestName + '.txt')
    $CodecTest = Start-Process -FilePath (Join-Path $ProjectRoot ("build\" + $CodecTestName + '.exe')) `
        -ArgumentList @('-o', ($CodecTestLog + ',txt')) -Wait -PassThru -WindowStyle Hidden
    if ($CodecTest.ExitCode -ne 0) {
        Get-Content -LiteralPath $CodecTestLog
        throw "Native codec test failed: $CodecTestName ($($CodecTest.ExitCode))."
    }
}
$SampleProject = Join-Path $ProjectRoot 'samples\下载链路验证.ksp'
# 20261108-12 change: MinGW argv can misdecode non-ASCII paths launched by Windows PowerShell.
# Validate a byte-identical ASCII-path copy; the private-desktop GUI probe separately opens the real Unicode path.
$ValidationSample = Join-Path $ProjectRoot 'build\validation-sample.ksp'
Copy-Item -LiteralPath $SampleProject -Destination $ValidationSample -Force
$RejectedDownload = Start-Process -FilePath $Application `
    -ArgumentList @('--download-native-test', $ValidationSample) `
    -Wait -PassThru -WindowStyle Hidden # 20261108-10 add: Reject unsupported layouts before claiming USB.
if ($RejectedDownload.ExitCode -ne 14) {
    throw "An unsupported project was not rejected before USB access: $($RejectedDownload.ExitCode)."
}
$SampleTest = Start-Process -FilePath $Application `
    -ArgumentList @('--validate-project', $ValidationSample) `
    -Wait -PassThru -WindowStyle Hidden
if ($SampleTest.ExitCode -ne 0) {
    throw "The sample project validation failed with code $($SampleTest.ExitCode)."
}
$NativeSample = Join-Path $ProjectRoot 'samples\native-device-test\CodexProbeTest.ksp'
$NativeExpected = Join-Path $ProjectRoot 'samples\native-device-test\CodexProbeTest.sm'
$SingleTextSamples = Get-ChildItem -LiteralPath (Join-Path $ProjectRoot 'samples\single-text') -Filter '*.ksp' -File # 20261108-12 change
$SampleIndex = 0
foreach ($SingleTextSample in $SingleTextSamples) {
    $ValidationSingleTextSample = Join-Path $ProjectRoot "build\single-text-sample-$SampleIndex.ksp"
    Copy-Item -LiteralPath $SingleTextSample.FullName -Destination $ValidationSingleTextSample -Force
    $SingleTextSampleTest = Start-Process -FilePath $Application `
        -ArgumentList @('--validate-project', $ValidationSingleTextSample) `
        -Wait -PassThru -WindowStyle Hidden
    if ($SingleTextSampleTest.ExitCode -ne 0) {
        throw "The single-text sample failed validation: $($SingleTextSample.FullName): $($SingleTextSampleTest.ExitCode)."
    }
    $SampleIndex++
}
$env:QT_QPA_PLATFORM = 'windows' # 20261108-11 change: Font raster checks need the real Windows font engine, without a visible window.
$StaticTextFeatureSamples = Get-ChildItem -LiteralPath (Join-Path $ProjectRoot 'samples\static-text') -Filter '*.ksp' -File # 20261108-14 add
$StaticCompileIndex = 0 #20261109-0 add
foreach ($StaticTextFeatureSample in $StaticTextFeatureSamples) {
    $FeatureValidationCopy = Join-Path $ProjectRoot 'build\static-text-feature-sample.ksp'
    Copy-Item -LiteralPath $StaticTextFeatureSample.FullName -Destination $FeatureValidationCopy -Force
    $FeatureValidation = Start-Process -FilePath $Application -ArgumentList @('--validate-project', $FeatureValidationCopy) `
        -Wait -PassThru -WindowStyle Hidden
    if ($FeatureValidation.ExitCode -ne 0) { throw "Static-text feature sample failed: $($StaticTextFeatureSample.Name)." }
    $FeatureCompileOutput = Join-Path $ProjectRoot ("build\static-text-compiled-" + $StaticCompileIndex) #20261109-0 add
    $FeatureCompile = Start-Process -FilePath $Application -ArgumentList @('--compile-project', $FeatureValidationCopy, $FeatureCompileOutput) -Wait -PassThru -WindowStyle Hidden
    if ($FeatureCompile.ExitCode -ne 0) { throw "Static-text offline compilation failed: $($StaticTextFeatureSample.Name)." }
    $StaticCompileIndex++
}
#20261108-15 add: Every button example must load and must be rejected before opening USB.
$BitButtonSamples = Get-ChildItem -LiteralPath (Join-Path $ProjectRoot 'samples\bit-button') -Filter '*.ksp' -File
$BitCompileIndex = 0 #20261109-0 add
foreach ($BitButtonSample in $BitButtonSamples) {
    $BitValidationCopy = Join-Path $ProjectRoot 'build\bit-button-validation.ksp'
    Copy-Item -LiteralPath $BitButtonSample.FullName -Destination $BitValidationCopy -Force
    $BitValidation = Start-Process -FilePath $Application -ArgumentList @('--validate-project', $BitValidationCopy) -Wait -PassThru -WindowStyle Hidden
    if ($BitValidation.ExitCode -ne 0) { throw "Bit-button sample validation failed: $($BitButtonSample.Name)." }
    $BitRefusal = Start-Process -FilePath $Application -ArgumentList @('--download-single-text', $BitValidationCopy) -Wait -PassThru -WindowStyle Hidden
    if ($BitRefusal.ExitCode -ne 14) { throw "Unsupported button sample was not rejected before USB: $($BitRefusal.ExitCode)." }
    $BitCompileOutput = Join-Path $ProjectRoot ("build\bit-button-compiled-" + $BitCompileIndex) #20261109-0 change: Keep CLI paths ASCII for MinGW argv.
    $BitCompile = Start-Process -FilePath $Application -ArgumentList @('--compile-project', $BitValidationCopy, $BitCompileOutput) -Wait -PassThru -WindowStyle Hidden #20261109-0 add
    if ($BitCompile.ExitCode -ne 0) { throw "Offline bit-button compilation failed: $($BitButtonSample.Name): $($BitCompile.ExitCode)." }
    $BitCompileIndex++
}
#20261109-6 add: Compile each word example and reopen its typed source before deployment.
$WordButtonSamples = Get-ChildItem -LiteralPath (Join-Path $ProjectRoot 'samples\word-button') -Filter '*.ksp' -File
$WordCompileIndex = 0
foreach ($WordButtonSample in $WordButtonSamples) {
    $WordValidationCopy = Join-Path $ProjectRoot 'build\word-button-validation.ksp'
    Copy-Item -LiteralPath $WordButtonSample.FullName -Destination $WordValidationCopy -Force
    $WordCompileOutput = Join-Path $ProjectRoot ("build\word-button-compiled-" + $WordCompileIndex)
    $WordCompile = Start-Process -FilePath $Application -ArgumentList @('--compile-project', $WordValidationCopy, $WordCompileOutput) -Wait -PassThru -WindowStyle Hidden
    if ($WordCompile.ExitCode -ne 0) { throw "Offline word-button compilation failed: $($WordButtonSample.Name): $($WordCompile.ExitCode)." }
    $WordNativeSource = Join-Path $ProjectRoot ("build\word-button-source-" + $WordCompileIndex + '.sm')
    $WordSourceExport = Start-Process -FilePath $Application -ArgumentList @('--export-native-project', $WordValidationCopy, $WordNativeSource) -Wait -PassThru -WindowStyle Hidden
    if ($WordSourceExport.ExitCode -ne 0) { throw "Word-button native source export failed: $($WordButtonSample.Name): $($WordSourceExport.ExitCode)." }
    $WordSourceCompileOutput = Join-Path $ProjectRoot ("build\word-button-source-compiled-" + $WordCompileIndex)
    $WordSourceCompile = Start-Process -FilePath $Application -ArgumentList @('--compile-project', $WordNativeSource, $WordSourceCompileOutput) -Wait -PassThru -WindowStyle Hidden
    if ($WordSourceCompile.ExitCode -ne 0) { throw "Word-button native source reopen compilation failed: $($WordButtonSample.Name): $($WordSourceCompile.ExitCode)." }
    $WordCompileIndex++
}
#20261109-8 add: Compile each lamp example and reopen its typed source before deployment.
$LampSamples = Get-ChildItem -LiteralPath (Join-Path $ProjectRoot 'samples\lamp') -Filter '*.ksp' -File
$LampCompileIndex = 0
foreach ($LampSample in $LampSamples) {
    $LampValidationCopy = Join-Path $ProjectRoot 'build\lamp-validation.ksp'
    Copy-Item -LiteralPath $LampSample.FullName -Destination $LampValidationCopy -Force
    $LampCompileOutput = Join-Path $ProjectRoot ("build\lamp-compiled-" + $LampCompileIndex)
    $LampCompile = Start-Process -FilePath $Application -ArgumentList @('--compile-project', $LampValidationCopy, $LampCompileOutput) -Wait -PassThru -WindowStyle Hidden
    if ($LampCompile.ExitCode -ne 0) { throw "Offline lamp compilation failed: $($LampSample.Name): $($LampCompile.ExitCode)." }
    $LampNativeSource = Join-Path $ProjectRoot ("build\lamp-source-" + $LampCompileIndex + '.sm')
    $LampSourceExport = Start-Process -FilePath $Application -ArgumentList @('--export-native-project', $LampValidationCopy, $LampNativeSource) -Wait -PassThru -WindowStyle Hidden
    if ($LampSourceExport.ExitCode -ne 0) { throw "Lamp-button native source export failed: $($LampSample.Name): $($LampSourceExport.ExitCode)." }
    $LampSourceCompileOutput = Join-Path $ProjectRoot ("build\lamp-source-compiled-" + $LampCompileIndex)
    $LampSourceCompile = Start-Process -FilePath $Application -ArgumentList @('--compile-project', $LampNativeSource, $LampSourceCompileOutput) -Wait -PassThru -WindowStyle Hidden
    if ($LampSourceCompile.ExitCode -ne 0) { throw "Lamp-button native source reopen compilation failed: $($LampSample.Name): $($LampSourceCompile.ExitCode)." }
    $LampCompileIndex++
}
#20261110-0 add: Compile fresh function projects and preserve their complete native-source round trip.
$FunctionButtonSamples = Get-ChildItem -LiteralPath (Join-Path $ProjectRoot 'samples\function-button') -Filter '*.ksp' -File
$FunctionCompileIndex = 0
foreach ($FunctionButtonSample in $FunctionButtonSamples) {
    $FunctionValidationCopy = Join-Path $ProjectRoot 'build\function-validation.ksp'
    Copy-Item -LiteralPath $FunctionButtonSample.FullName -Destination $FunctionValidationCopy -Force
    $FunctionCompileOutput = Join-Path $ProjectRoot ("build\function-compiled-" + $FunctionCompileIndex)
    $FunctionCompile = Start-Process -FilePath $Application -ArgumentList @('--compile-project', $FunctionValidationCopy, $FunctionCompileOutput) -Wait -PassThru -WindowStyle Hidden
    if ($FunctionCompile.ExitCode -ne 0) { throw "Offline function-button compilation failed: $($FunctionButtonSample.Name): $($FunctionCompile.ExitCode)." }
    $FunctionNativeSource = Join-Path $ProjectRoot ("build\function-source-" + $FunctionCompileIndex + '.sm')
    $FunctionSourceExport = Start-Process -FilePath $Application -ArgumentList @('--export-native-project', $FunctionValidationCopy, $FunctionNativeSource) -Wait -PassThru -WindowStyle Hidden
    if ($FunctionSourceExport.ExitCode -ne 0) { throw "Function native source export failed: $($FunctionButtonSample.Name): $($FunctionSourceExport.ExitCode)." }
    $FunctionSourceCompileOutput = Join-Path $ProjectRoot ("build\function-source-compiled-" + $FunctionCompileIndex)
    $FunctionSourceCompile = Start-Process -FilePath $Application -ArgumentList @('--compile-project', $FunctionNativeSource, $FunctionSourceCompileOutput) -Wait -PassThru -WindowStyle Hidden
    if ($FunctionSourceCompile.ExitCode -ne 0) { throw "Function native source reopen failed: $($FunctionButtonSample.Name): $($FunctionSourceCompile.ExitCode)." }
    $FunctionSecondNative = Join-Path $ProjectRoot ("build\function-source-second-" + $FunctionCompileIndex + '.sm')
    $FunctionReExport = Start-Process -FilePath $Application -ArgumentList @('--export-native-project', $FunctionNativeSource, $FunctionSecondNative) -Wait -PassThru -WindowStyle Hidden
    if ($FunctionReExport.ExitCode -ne 0 -or (Get-FileHash -LiteralPath $FunctionNativeSource).Hash -ne (Get-FileHash -LiteralPath $FunctionSecondNative).Hash) {
        throw "Function native source re-export differs: $($FunctionButtonSample.Name)."
    }
    $FunctionCompileIndex++
}
$CompilerTest = Start-Process -FilePath $Application `
    -ArgumentList @('--verify-single-text-compiler', $NativeSample) `
    -Wait -PassThru -WindowStyle Hidden # 20261108-11 add: Compare AB12, shifted AB12, and AAAA against native RGB565 digests.
if ($CompilerTest.ExitCode -ne 0) {
    throw "The native single-text raster regression failed with code $($CompilerTest.ExitCode)."
}
$env:QT_QPA_PLATFORM = 'offscreen'
$NativeImportTest = Start-Process -FilePath $Application `
    -ArgumentList @('--validate-project', $NativeExpected) `
    -Wait -PassThru -WindowStyle Hidden # 20261108-9 add: Import the real native sample through the production loader.
if ($NativeImportTest.ExitCode -ne 0) {
    throw "The verified native project import failed with code $($NativeImportTest.ExitCode)."
}
$NativeOutput = Join-Path $ProjectRoot 'build\native-export-test.sm'
$NativeTest = Start-Process -FilePath $Application `
    -ArgumentList @('--export-native-test', $NativeSample, $NativeOutput) `
    -Wait -PassThru -WindowStyle Hidden
if ($NativeTest.ExitCode -ne 0) {
    throw "The native source export failed with code $($NativeTest.ExitCode)."
}
if ((Get-FileHash -Algorithm SHA256 -LiteralPath $NativeOutput).Hash -ne `
    (Get-FileHash -Algorithm SHA256 -LiteralPath $NativeExpected).Hash) {
    throw 'The native source export differs from the vendor-opened reference.'
}
$NativeRoundTrip = Join-Path $ProjectRoot 'build\native-import-roundtrip.sm'
$RoundTripTest = Start-Process -FilePath $Application `
    -ArgumentList @('--export-native-test', $NativeExpected, $NativeRoundTrip) `
    -Wait -PassThru -WindowStyle Hidden # 20261108-9 add: Preserve all verified binary bytes after import/export.
if ($RoundTripTest.ExitCode -ne 0 -or
    (Get-FileHash -Algorithm SHA256 -LiteralPath $NativeRoundTrip).Hash -ne `
    (Get-FileHash -Algorithm SHA256 -LiteralPath $NativeExpected).Hash) {
    throw 'Native import/export did not preserve the verified source.'
}
$NativeInvalid = Join-Path $ProjectRoot 'build\native-invalid-test.sm'
$InvalidBytes = [System.IO.File]::ReadAllBytes($NativeExpected)
$InvalidBytes[0] = 0x7f #20261109-0 change: Corrupt the bounded stream length; valid opaque metadata edits may now import safely.
[System.IO.File]::WriteAllBytes($NativeInvalid, $InvalidBytes)
$InvalidTest = Start-Process -FilePath $Application `
    -ArgumentList @('--validate-project', $NativeInvalid) `
    -Wait -PassThru -WindowStyle Hidden # 20261108-9 add: Reject a same-size native file with an unverified layout.
if ($InvalidTest.ExitCode -ne 7) {
    throw "A malformed native stream did not return the expected validation error: $($InvalidTest.ExitCode)."
}

if ($Deploy) {
    $env:QT_QPA_PLATFORM = 'windows' # 20261108-7 change: Deployment contains qwindows and tests display no windows.
    $DeployDirectory = Join-Path $ProjectRoot ('dist\' + $DeployDirectoryName) #20261109-1 change
    $Windeployqt = Join-Path $QtRoot 'bin\windeployqt.exe'
    $SampleDirectory = Join-Path $DeployDirectory 'samples'
    $GuideDirectory = Join-Path $DeployDirectory 'docs' #20261109-0 add
    $NativeSampleDirectory = Join-Path $SampleDirectory 'native-device-test'
    $SingleTextSampleDirectory = Join-Path $SampleDirectory 'single-text' # 20261108-11 add
    $StaticTextSampleDirectory = Join-Path $SampleDirectory 'static-text' # 20261108-14 add
    New-Item -ItemType Directory -Force -Path $DeployDirectory | Out-Null
    New-Item -ItemType Directory -Force -Path $SampleDirectory | Out-Null
    New-Item -ItemType Directory -Force -Path $GuideDirectory | Out-Null
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'README.md') -Destination $DeployDirectory -Force
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'docs\位按钮手册实现与验收.md') -Destination $GuideDirectory -Force
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'docs\位按钮设备编译实现.md') -Destination $GuideDirectory -Force
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'docs') -Destination $DeployDirectory -Recurse -Force #20261109-0 add: Include linked protocol and historical acceptance documents.
    $DesignGuideDirectory = Join-Path $DeployDirectory 'design_plan'
    New-Item -ItemType Directory -Force -Path $DesignGuideDirectory | Out-Null
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'design_plan\位按钮功能与交互复刻手册.md') -Destination $DesignGuideDirectory -Force
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'design_plan\位按钮复刻手册.assets') -Destination $DesignGuideDirectory -Recurse -Force
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'design_plan\静态文本功能与交互复刻手册.md') -Destination $DesignGuideDirectory -Force #20261109-0 add
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'design_plan\静态文本复刻手册.assets') -Destination $DesignGuideDirectory -Recurse -Force
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'design_plan\字按钮功能与交互复刻手册.md') -Destination $DesignGuideDirectory -Force #20261109-6 add
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'design_plan\字按钮复刻手册.assets') -Destination $DesignGuideDirectory -Recurse -Force
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'design_plan\指示灯功能与交互复刻手册.md') -Destination $DesignGuideDirectory -Force #20261109-8 add
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'design_plan\指示灯复刻手册.assets') -Destination $DesignGuideDirectory -Recurse -Force
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'design_plan\功能按钮功能与交互复刻手册.md') -Destination $DesignGuideDirectory -Force #20261110-0 add
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'design_plan\功能按钮复刻手册.assets') -Destination $DesignGuideDirectory -Recurse -Force
    foreach ($Stage1Guide in @('02_阶段1_工程模型与文件格式.md', 'A05_源格式与设备编译契约.md', '03_阶段2_主窗口与工程配置.md', 'A02_全局入口与工程配置规范.md')) { #20261110-7 change: Include the workflow contract and its explicit implementation boundaries.
        Copy-Item -LiteralPath (Join-Path $ProjectRoot ('design_plan\' + $Stage1Guide)) -Destination $DesignGuideDirectory -Force
    }
    $Stage1FixtureDirectory = Join-Path $DeployDirectory 'tests\fixtures'
    New-Item -ItemType Directory -Force -Path $Stage1FixtureDirectory | Out-Null
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'tests\fixtures\native-source') -Destination $Stage1FixtureDirectory -Recurse -Force
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'tests\fixtures\lamp-control') -Destination $Stage1FixtureDirectory -Recurse -Force #20261110-7 add: Preserve durable native recompiled regression oracles.
    $PreviewDirectory = Join-Path $DeployDirectory 'artifacts'
    New-Item -ItemType Directory -Force -Path $PreviewDirectory | Out-Null
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'artifacts\KinSealStudioLite-preview.png') -Destination $PreviewDirectory -Force #20261109-0 add: Preserve the historical verification report's image link.
    Get-ChildItem -LiteralPath (Join-Path $ProjectRoot 'artifacts') -Filter '*.png' -File | Copy-Item -Destination $PreviewDirectory -Force #20261110-0 add: Preserve gallery images linked by the packaged README.
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'artifacts\word-button') -Destination $PreviewDirectory -Recurse -Force #20261109-6 add: Include private-Windows word-dialog evidence referenced by the guide.
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'artifacts\lamp') -Destination $PreviewDirectory -Recurse -Force #20261109-8 add: Include isolated Windows lamp-dialog evidence.
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'artifacts\function-button') -Destination $PreviewDirectory -Recurse -Force #20261110-0 add
    New-Item -ItemType Directory -Force -Path $NativeSampleDirectory | Out-Null
    New-Item -ItemType Directory -Force -Path $SingleTextSampleDirectory | Out-Null
    New-Item -ItemType Directory -Force -Path $StaticTextSampleDirectory | Out-Null
    Copy-Item -LiteralPath $Application -Destination $DeployDirectory -Force
    Copy-Item -LiteralPath $SampleProject -Destination $SampleDirectory -Force
    Copy-Item -LiteralPath $NativeSample -Destination $NativeSampleDirectory -Force
    Copy-Item -LiteralPath $NativeExpected -Destination $NativeSampleDirectory -Force # 20261108-9 add: Include the native file users can now open.
    foreach ($SingleTextSample in $SingleTextSamples) {
        Copy-Item -LiteralPath $SingleTextSample.FullName -Destination $SingleTextSampleDirectory -Force # 20261108-12 change
    }
    foreach ($StaticTextFeatureSample in $StaticTextFeatureSamples) {
        Copy-Item -LiteralPath $StaticTextFeatureSample.FullName -Destination $StaticTextSampleDirectory -Force
    }
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'samples\bit-button') -Destination $SampleDirectory -Recurse -Force #20261108-15 add
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'samples\word-button') -Destination $SampleDirectory -Recurse -Force #20261109-6 add
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'samples\lamp') -Destination $SampleDirectory -Recurse -Force #20261109-8 add
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'samples\function-button') -Destination $SampleDirectory -Recurse -Force #20261110-0 add
    foreach ($StageSampleName in @('stage0', 'stage1', 'stage2')) { #20261110-7 change: Ship workflow-only and downloadable examples with explicit boundaries.
        if (Test-Path -LiteralPath (Join-Path $ProjectRoot ('samples\' + $StageSampleName))) {
            Copy-Item -LiteralPath (Join-Path $ProjectRoot ('samples\' + $StageSampleName)) -Destination $SampleDirectory -Recurse -Force
        }
    }
    Copy-Item -LiteralPath (Join-Path $ProjectRoot 'third_party\lzokay\LICENSE') `
        -Destination (Join-Path $DeployDirectory 'LZO-MIT-LICENSE.txt') -Force # 20261108-11 add: Ship the bundled codec attribution.
    & $Windeployqt --release --compiler-runtime --no-translations --dir $DeployDirectory `
        (Join-Path $DeployDirectory 'KinSealStudioLite.exe')
    if ($LASTEXITCODE -ne 0) {
        throw "windeployqt failed with code $LASTEXITCODE."
    }
    $DeployedApplication = Join-Path $DeployDirectory 'KinSealStudioLite.exe'
    $DeployedPayloadTest = Start-Process -FilePath $DeployedApplication `
        -ArgumentList '--verify-independent-payload' -Wait -PassThru -WindowStyle Hidden # 20261108-10 add
    if ($DeployedPayloadTest.ExitCode -ne 0) {
        throw "The deployed independent TEST payload failed verification: $($DeployedPayloadTest.ExitCode)."
    }
    $DeployedSample = Join-Path $SampleDirectory '下载链路验证.ksp'
    $DeployedValidationSample = Join-Path $ProjectRoot 'build\deployed-validation-sample.ksp'
    Copy-Item -LiteralPath $DeployedSample -Destination $DeployedValidationSample -Force # 20261108-12 change: Exercise the packaged file via an ASCII launch path.
    $PackageTest = Start-Process -FilePath $DeployedApplication `
        -ArgumentList '--self-test' -Wait -PassThru -WindowStyle Hidden
    if ($PackageTest.ExitCode -ne 0) {
        throw "The deployed project round-trip test failed with code $($PackageTest.ExitCode)."
    }
    $PackageSampleTest = Start-Process -FilePath $DeployedApplication `
        -ArgumentList @('--validate-project', $DeployedValidationSample) `
        -Wait -PassThru -WindowStyle Hidden
    if ($PackageSampleTest.ExitCode -ne 0) {
        throw "The deployed sample project validation failed with code $($PackageSampleTest.ExitCode)."
    }
    $SampleIndex = 0
    foreach ($SingleTextSample in $SingleTextSamples) {
        $DeployedSingleTextSample = Join-Path $SingleTextSampleDirectory $SingleTextSample.Name
        $DeployedValidationSingleText = Join-Path $ProjectRoot "build\deployed-single-text-$SampleIndex.ksp"
        Copy-Item -LiteralPath $DeployedSingleTextSample -Destination $DeployedValidationSingleText -Force
        $DeployedSingleTextTest = Start-Process -FilePath $DeployedApplication `
            -ArgumentList @('--validate-project', $DeployedValidationSingleText) `
            -Wait -PassThru -WindowStyle Hidden
        if ($DeployedSingleTextTest.ExitCode -ne 0) {
            throw "The deployed single-text sample failed validation: $($SingleTextSample.Name): $($DeployedSingleTextTest.ExitCode)."
        }
        $SampleIndex++
    }
    $DeployedNative = Join-Path $NativeSampleDirectory 'CodexProbeTest.sm'
    $PackageCompilerTest = Start-Process -FilePath $DeployedApplication `
        -ArgumentList @('--verify-single-text-compiler', (Join-Path $NativeSampleDirectory 'CodexProbeTest.ksp')) `
        -Wait -PassThru -WindowStyle Hidden # 20261108-11 add: Check the Windows-font compiler in the portable package.
    if ($PackageCompilerTest.ExitCode -ne 0) {
        throw "The deployed single-text compiler failed verification: $($PackageCompilerTest.ExitCode)."
    }
    $PackageNativeTest = Start-Process -FilePath $DeployedApplication `
        -ArgumentList @('--validate-project', $DeployedNative) `
        -Wait -PassThru -WindowStyle Hidden # 20261108-9 add: Verify the packaged native sample uses the production importer.
    if ($PackageNativeTest.ExitCode -ne 0) {
        throw "The deployed native project validation failed with code $($PackageNativeTest.ExitCode)."
    }
}

Write-Output "Build and self-test completed: $Application"
