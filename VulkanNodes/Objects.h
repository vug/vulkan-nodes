#pragma once

namespace ne {
	struct MyStruct {
		int count;
		float magnitude;
	};

	enum class YourEnum {
		Opt1,
		Opt2,
	};

	struct YourStruct {
		YourEnum option;
		int num;
	};
}