#pragma once
#include "Windows.h"

#include <string>

namespace NoEngine {
	class WindowManager {
	public:
		void Initialize(std::wstring titleName);

		bool ProcessMessage();

	private:
		
	};

}
