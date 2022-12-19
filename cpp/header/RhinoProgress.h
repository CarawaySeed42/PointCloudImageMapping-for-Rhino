#pragma once

class RhinoProgress {

public:
	static inline void PromptProgress(const char message[], const size_t curIteration, const size_t totalCount) {

		char printBuf[150];
		snprintf(printBuf, 145, "%s...%4.2f%%", message, ((double)curIteration / (double)totalCount) * 100);
		const WCHAR* pwcsPrint;
		int nChars = MultiByteToWideChar(CP_ACP, 0, printBuf, -1, NULL, 0);
		pwcsPrint = new WCHAR[nChars];
		MultiByteToWideChar(CP_ACP, 0, printBuf, -1, (LPWSTR)pwcsPrint, nChars);
		RhinoApp().SetCommandPrompt(pwcsPrint);
		RhinoApp().Wait(0);
		delete[] pwcsPrint;
	}

	static inline void PromptMessage(const char message[]) {

		char printBuf[150];
		snprintf(printBuf, 145, "%s", message);
		const WCHAR* pwcsPrint;
		int nChars = MultiByteToWideChar(CP_ACP, 0, printBuf, -1, NULL, 0);
		pwcsPrint = new WCHAR[nChars];
		MultiByteToWideChar(CP_ACP, 0, printBuf, -1, (LPWSTR)pwcsPrint, nChars);
		RhinoApp().SetCommandPromptMessage(pwcsPrint);
		RhinoApp().Wait(0);
		delete[] pwcsPrint;
	}
};
