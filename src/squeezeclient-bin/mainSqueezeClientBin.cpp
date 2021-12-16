#include "SqueezeClientBin.h"

#include <cpp-app-utils/Logger.h>

using namespace CppAppUtils;
using namespace squeezeclient;

int main(int argc, char* argv[])
{
	int result;

	//TODO: Change sort of
//	Logger::SetLogLevel(Logger::DEBUG);
	Logger::SetLogLevel(Logger::INFO);

	SqueezeClientBin* clientInstance=SqueezeClientBin::Instance();
	if (clientInstance->Init(argc,argv))
		clientInstance->Run();

	result=clientInstance->GetReturnCode();

	clientInstance->DeInit();

	return result;

	return 0;
}
