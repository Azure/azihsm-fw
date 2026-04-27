param(
    [Parameter(Mandatory = $true)]
    [string]$rc_version,
    
    [Parameter(Mandatory = $true)]
    [string]$is_fips_image,

    [string] $build_ext
)

Write-Host "Input rc_version: $rc_version"
Write-Host "Input is_fips_image: $is_fips_image"

$baseVersion = if ($is_fips_image) { $rc_version -replace '-fips$','' } else { $rc_version }

if ($baseVersion -notmatch '^\d+\.\d+\.\d+$') {
    throw "rc_version format invalid. Expected format: major.minor.patch (e.g. 4.3.7)"
}

$universalVersion = "3.$rc_version"
Write-Host "Derived universalVersion: $universalVersion"

Write-Host "##vso[task.setvariable variable=universalVersion;issecret=false]$universalVersion"


$suffix = if ($is_fips_image) { "-fips" } else { "" }


if ($is_fips_image.Trim().ToLower() -eq "true") {
    $manticore_fipsrel_version = "manticore_v3.$baseVersion" + $build_ext +"$suffix" + ".bin"

    Write-Host "Manticore fips rel image name: $manticore_fipsrel_version"
    Write-Host "##vso[task.setvariable variable=manticore_fipsrel_version;issecret=false]$manticore_fipsrel_version"
    $manticore_image = $manticore_fipsrel_version
    Write-Host "##vso[task.setvariable variable=fipsArg]--fips"
} 
else 
{
    $manticore_image = "manticore_v$universalVersion$build_ext.bin"
    Write-Host "##vso[task.setvariable variable=fipsArg]"
}

Write-Host "Final image name: $manticore_image"

# Set pipeline variable for later tasks
Write-Host "##vso[task.setvariable variable=manticore_image]$manticore_image"

