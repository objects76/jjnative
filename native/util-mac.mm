
#import <Foundation/Foundation.h>
#import <Foundation/NSProcessInfo.h>

bool oc_getSystemVersion(int32_t* majorVersion, int32_t* minorVersion, int32_t* patchedVersion)
{
	NSProcessInfo *processInfo = [[NSProcessInfo alloc] init];

	// check avaiability of the property operatingSystemVersion (10.10+) at runtime
	if ([processInfo respondsToSelector:@selector(operatingSystemVersion)])
	{
		NSOperatingSystemVersion versionObj = [processInfo operatingSystemVersion];
		*majorVersion = (int32_t)versionObj.majorVersion;
		*minorVersion = (int32_t)versionObj.minorVersion;
		*patchedVersion = (int32_t)versionObj.patchVersion;
		return true;
	}
	else
	{
		return false;
		// Implement fallback for OSX 10.9 and below
	}
}
