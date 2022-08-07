//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#pragma once

#include <string>
#include <vector>

namespace Kimura
{
	class IKimuraConverter
	{
		public:

			virtual ~IKimuraConverter() {}

			virtual void Start() = 0;
			virtual void Stop() = 0;

			virtual volatile bool IsWorking() = 0;

			virtual bool HasSucceeded() = 0;

			virtual std::string GetErrorMessage() = 0;

			virtual void GetConversionProgress(int& OutNumFramesWritten, int& OutNumFramesTotal) = 0;

	};


	IKimuraConverter* CreateConverter(std::vector<std::string>& InArgs);

}