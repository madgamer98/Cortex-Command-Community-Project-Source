#include "RTETools.h"
#include "Vector.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SeedRand() { srand(time(0)); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double PosRand() { return (rand() / (RAND_MAX / 1000 + 1)) / 1000.0; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double NormalRand() { return (static_cast<double>(rand()) / (RAND_MAX / 2)) - 1.0; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double RangeRand(float min, float max) { return min + ((max - min) * PosRand()); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SelectRand(int min, int max) { return min + static_cast<int>((max - min) * PosRand() + 0.5); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float LERP(float xStart, float xEnd, float yStart, float yEnd, float progressScalar) {
		if (progressScalar <= xStart) {
			return yStart;
		} else if (progressScalar >= xEnd) {
			return yEnd;
		}
		return yStart + ((progressScalar - xStart) * ((yEnd - yStart) / (xEnd - xStart)));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float EaseIn(float start, float end, float progressScalar) {
		if (progressScalar <= 0) {
			return start;
		} else if (progressScalar >= 1.0) {
			return end;
		}
		float t = 1 - progressScalar;
		return (end - start) * (sinf(-t * c_HalfPI) + 1) + start;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float EaseOut(float start, float end, float progressScalar) {
		if (progressScalar <= 0) {
			return start;
		} else if (progressScalar >= 1.0) {
			return end;
		}
		return (end - start) * -sinf(-progressScalar * c_HalfPI) + start;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float EaseInOut(float start, float end, float progressScalar) {
		return start * (2 * powf(progressScalar, 3) - 3 * powf(progressScalar, 2) + 1) + end * (3 * powf(progressScalar, 2) - 2 * powf(progressScalar, 3));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Clamp(float &value, float upperLimit, float lowerLimit) {
		// Straighten out the limits
		if (upperLimit < lowerLimit) {
			float temp = upperLimit;
			upperLimit = lowerLimit;
			lowerLimit = temp;
		}
		// Do the clamping
		if (value > upperLimit) {
			value = upperLimit;
			return true;
		} else if (value < lowerLimit) {
			value = lowerLimit;
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float Limit(float value, float upperLimit, float lowerLimit) {
		// Straighten out the limits
		if (upperLimit < lowerLimit) {
			float temp = upperLimit;
			upperLimit = lowerLimit;
			lowerLimit = temp;
		}

		// Do the clamping
		if (value > upperLimit) {
			return upperLimit;
		} else if (value < lowerLimit) {
			return lowerLimit;
		}
		return value;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool WithinBox(Vector &point, float left, float top, float right, float bottom) {
		return point.m_X >= left && point.m_X < right && point.m_Y >= top && point.m_Y < bottom;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool WithinBox(Vector &point, Vector &boxPos, float width, float height) {
		return point.m_X >= boxPos.m_X && point.m_X < (boxPos.m_X + width) && point.m_Y >= boxPos.m_Y && point.m_Y < (boxPos.m_Y + height);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void OpenBrowserToURL(std::string goToURL) {
		system(std::string("start ").append(goToURL).c_str());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ASCIIFileContainsString(std::string filePath, std::string findString) {
		// Open the script file so we can check it out
		std::ifstream *pFile = new std::ifstream(filePath.c_str());
		if (!pFile->good()) {
			return false;
		}
		char rawLine[1024];
		std::string line;
		std::string::size_type pos = 0;
		std::string::size_type endPos = 0;
		std::string::size_type commentPos = std::string::npos;
		bool blockCommented = false;

		while (!pFile->eof()) {
			// Go through the script file, line by line
			pFile->getline(rawLine, 1024);
			line = rawLine;
			pos = endPos = 0;
			commentPos = std::string::npos;

			// Check for block comments
			if (!blockCommented && (commentPos = line.find("/*", 0)) != std::string::npos) { blockCommented = true; }

			// Find the end of the block comment
			if (blockCommented) {
				if ((commentPos = line.find("*/", commentPos == std::string::npos ? 0 : commentPos)) != std::string::npos) {
					blockCommented = false;
					pos = commentPos;
				}
			}
			// Process the line as usual
			if (!blockCommented) {
				// See if this line is commented out anywhere
				commentPos = line.find("//", 0);
				// Find the string
				do {
					pos = line.find(findString.c_str(), pos);
					if (pos != std::string::npos && pos < commentPos) {
						// Found it!
						delete pFile;
						pFile = 0;
						return true;
					}
				} while (pos != std::string::npos && pos < commentPos);
			}
		}
		// Didn't find the search string
		delete pFile;
		pFile = 0;
		return false;
	}
}