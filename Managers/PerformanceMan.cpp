#include "PerformanceMan.h"
#include "FrameMan.h"
#include "AudioMan.h"
#include "Timer.h"

#include "GUI.h"
#include "AllegroBitmap.h"

namespace RTE {

	const std::string PerformanceMan::m_ClassName = "PerformanceMan";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::Clear() {
		m_ShowPerfStats = false;
		m_CurrentPing = 0;
		m_FrameTimer = 0;
		m_MSPFs.clear();
		m_MSPFAverage = 0;
		m_SimSpeed = 1.0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PerformanceMan::Create() {
		m_FrameTimer = new Timer();
		m_Sample = 0;

		for (unsigned short c = 0; c < PERF_COUNT; ++c) {
			for (unsigned short i = 0; i < c_MaxSamples; ++i) {
				m_PerfData[c][i] = 0;
				m_PerfPercentages[c][i] = 0;
			}
			m_PerfMeasureStart[c] = 0;
			m_PerfMeasureStop[c] = 0;
		}
		// Set up performance counter's names
		m_PerfCounterNames[PERF_SIM_TOTAL] = "Total";
		m_PerfCounterNames[PERF_ACTORS_PASS1] = "Act Travel";
		m_PerfCounterNames[PERF_PARTICLES_PASS1] = "Prt Travel";
		m_PerfCounterNames[PERF_ACTORS_PASS2] = "Act Update";
		m_PerfCounterNames[PERF_PARTICLES_PASS2] = "Prt Update";
		m_PerfCounterNames[PERF_ACTORS_AI] = "Act AI";
		m_PerfCounterNames[PERF_ACTIVITY] = "Activity";
		m_PerfCounterNames[PERF_SOUND] = "Sound";

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::Destroy() {
		delete m_FrameTimer;
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::ResetFrameTimer() { m_FrameTimer->Reset(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::StartPerformanceMeasurement(PerformanceCounters counter) { m_PerfMeasureStart[counter] = g_TimerMan.GetAbsoulteTime(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::StopPerformanceMeasurement(PerformanceCounters counter) {
		m_PerfMeasureStop[counter] = g_TimerMan.GetAbsoulteTime();
		AddPerformanceSample(counter, m_PerfMeasureStop[counter] - m_PerfMeasureStart[counter]);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::NewPerformanceSample() {
		m_Sample++;
		if (m_Sample >= c_MaxSamples) { m_Sample = 0; }

		for (unsigned short pc = 0; pc < PERF_COUNT; ++pc) {
			m_PerfData[pc][m_Sample] = 0;
			m_PerfPercentages[pc][m_Sample] = 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::Update() {
		// Time and add the millisecs per frame reading to the buffer
		m_MSPFs.push_back(m_FrameTimer->GetElapsedRealTimeMS());
		// Keep the buffer trimmed
		while (m_MSPFs.size() > c_MSPFAverageSampleSize) {
			m_MSPFs.pop_front();
		}
		// Calculate the average milliseconds per frame over the last sampleSize frames
		m_MSPFAverage = 0;
		for (std::deque<unsigned int>::iterator fItr = m_MSPFs.begin(); fItr != m_MSPFs.end(); ++fItr) {
			m_MSPFAverage += *fItr;
		}
		m_MSPFAverage /= m_MSPFs.size();
		
		// If one sim update per frame mode, adjust the pitch of most sound effects to match the sim time over real time ratio as it fluctuates!
		if (g_TimerMan.IsOneSimUpdatePerFrame()) {
			// Calculate the sim speed over the actual real time
			m_SimSpeed = g_TimerMan.GetDeltaTimeMS() / static_cast<float>(m_MSPFAverage);
			//        float simSpeed = g_TimerMan.GetDeltaTimeSecs() / m_FrameTimer->GetElapsedRealTimeS();
					// If limited, only allow pitch to go slower, not faster
			if (g_TimerMan.IsSimSpeedLimited() && m_SimSpeed > 1.0) {
				m_SimSpeed = 1.0;
			}
			// Soften the ratio of the pitch adjustment so it's not such an extreme effect on the audio
			// TODO: Don't hardcode this coefficient - although it's a good default
			float pitch = m_SimSpeed + (1.0F - m_SimSpeed) * 0.35;
			// Set the pitch for all other applicable sounds other than music
			g_AudioMan.SetGlobalPitch(pitch, true);
		} else {
			m_SimSpeed = 1.0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::Draw(AllegroBitmap bitmapToDrawTo ) {
		if (m_ShowPerfStats) {
			// Time and add the millisecs per frame reading to the buffer
			m_MSPFs.push_back(m_FrameTimer->GetElapsedRealTimeMS());
			m_FrameTimer->Reset();
			// Keep the buffer trimmed
			while (m_MSPFs.size() > c_MSPFAverageSampleSize) {
				m_MSPFs.pop_front();
			}		
			// Calculate the average milliseconds per frame over the last sampleSize frames
			//unsigned short m_MSPFAverage = 0;
			for (deque<unsigned int>::iterator fItr = m_MSPFs.begin(); fItr != m_MSPFs.end(); ++fItr) {
				m_MSPFAverage += *fItr;
			}
			m_MSPFAverage /= m_MSPFs.size();

			char str[512];
			
			// Calculate the fps from the average
			float fps = 1.0F / (static_cast<float>(m_MSPFAverage) / 1000.0F);
			sprintf_s(str, sizeof(str), "FPS: %.0f", fps);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, 17, 14, str, GUIFont::Left);

			// Display the average
			sprintf_s(str, sizeof(str), "MSPF: %i", m_MSPFAverage);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, 17, 24, str, GUIFont::Left);

			sprintf_s(str, sizeof(str), "Time Scale: x%.2f ([1]-, [2]+)", g_TimerMan.IsOneSimUpdatePerFrame() ? m_SimSpeed : g_TimerMan.GetTimeScale());
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, 17, 34, str, GUIFont::Left);

			sprintf_s(str, sizeof(str), "Real to Sim Cap: %.2f ms ([3]-, [4]+)", g_TimerMan.GetRealToSimCap() * 1000.0F);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, 17, 44, str, GUIFont::Left);

			float deltaTime = g_TimerMan.GetDeltaTimeMS();
			sprintf_s(str, sizeof(str), "DeltaTime: %.2f ms ([5]-, [6]+)", deltaTime);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, 17, 54, str, GUIFont::Left);

			sprintf_s(str, sizeof(str), "Particles: %i", g_MovableMan.GetParticleCount());
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, 17, 64, str, GUIFont::Left);

			sprintf_s(str, sizeof(str), "Objects: %i", g_MovableMan.GetKnownObjectsCount());
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, 17, 74, str, GUIFont::Left);

			sprintf_s(str, sizeof(str), "MOIDs: %i", g_MovableMan.GetMOIDCount());
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, 17, 84, str, GUIFont::Left);

			sprintf_s(str, sizeof(str), "Sim Updates Since Last Drawn: %i", g_TimerMan.SimUpdatesSinceDrawn());
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, 17, 94, str, GUIFont::Left);

			if (g_TimerMan.IsOneSimUpdatePerFrame()) { g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, 17, 104, "ONE Sim Update Per Frame!", GUIFont::Left); }

			sprintf_s(str, sizeof(str), "Sound channels: %d / %d ", g_AudioMan.GetPlayingChannelCount(), g_AudioMan.GetTotalChannelCount());
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, 17, 114, str, GUIFont::Left);

			unsigned short xOffset = 17;
			unsigned short yOffset = 134;
			unsigned short blockHeight = 34;
			unsigned short graphHeight = 20;
			unsigned short graphOffset = 14;

			// Update current sample percentage
			CalculateSamplePercentages();

			//Draw advanced performance counters
			for (unsigned short pc = 0; pc < PERF_COUNT; ++pc) {
				unsigned short blockStart = yOffset + pc * blockHeight;

				g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, xOffset, blockStart, m_PerfCounterNames[pc], GUIFont::Left);

				// Print percentage from PerformanceCounters::PERF_SIM_TOTAL
				unsigned short perc = static_cast<unsigned short>((static_cast<float>(GetPerormanceCounterAverage(static_cast<PerformanceCounters>(pc))) / static_cast<float>(GetPerormanceCounterAverage(PERF_SIM_TOTAL)) * 100));
				sprintf_s(str, sizeof(str), "%%: %u", perc);
				g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, xOffset + 60, blockStart, str, GUIFont::Left);

				// Print average processing time in ms
				sprintf_s(str, sizeof(str), "T: %lli", GetPerormanceCounterAverage(static_cast<PerformanceCounters>(pc)) / 1000);
				g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, xOffset + 96, blockStart, str, GUIFont::Left);

				unsigned short graphStart = blockStart + graphOffset;

				//Draw graph backgrounds
				bitmapToDrawTo.DrawRectangle(xOffset, graphStart, c_MaxSamples, graphHeight, 240, true);
				bitmapToDrawTo.DrawLine(xOffset, graphStart + graphHeight / 2, xOffset + c_MaxSamples, graphStart + graphHeight / 2, 96);
		
				//Reset peak value
				unsigned short peak = 0;

				//Draw sample dots
				unsigned int smpl = m_Sample;
				for (unsigned short i = 0; i < c_MaxSamples; i++) {
					if (smpl == 0) { smpl = c_MaxSamples; }
						
					// Show microseconds in graphs, assume that 33333 microseconds (one frame of 30 fps) is the highest value on the graph
					unsigned short value = static_cast<unsigned short>(static_cast<float>(m_PerfData[pc][smpl]) / (1000000 / 30) * 100);
					value = Limit(value, 100, 0);
					// Calculate dot height on the graph
					unsigned short dotHeight = static_cast<unsigned short>(static_cast<float>(graphHeight) / 100.0 * static_cast<float>(value));
					bitmapToDrawTo.SetPixel(xOffset + c_MaxSamples - i, graphStart + graphHeight - dotHeight, 13);
					peak = Limit(peak, m_PerfData[pc][smpl], 0);
					//Move to previous sample
					smpl--;
				}
				// Print peak values
				sprintf_s(str, sizeof(str), "Peak: %i", peak / 1000);
				g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, xOffset + 130, blockStart, str, GUIFont::Left);
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::DrawCurrentPing() {
		AllegroBitmap allegroBitmap(g_FrameMan.GetBackBuffer8());
		char buf[32];
		sprintf_s(buf, sizeof(buf), "PING: %u", m_CurrentPing);
		g_FrameMan.GetLargeFont()->DrawAligned(&allegroBitmap, g_FrameMan.GetBackBuffer8()->w - 25, g_FrameMan.GetBackBuffer8()->h - 14, buf, GUIFont::Right);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::CalculateSamplePercentages() {
		for (unsigned short pc = 0; pc < PERF_COUNT; ++pc) {
			unsigned short perc = static_cast<unsigned int>(static_cast<float>(m_PerfData[pc][m_Sample]) / static_cast<float>(m_PerfData[pc][PERF_SIM_TOTAL]) * 100);
			m_PerfPercentages[pc][m_Sample] = perc;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	uint64_t PerformanceMan::GetPerormanceCounterAverage(PerformanceCounters counter) {
		uint64_t accum = 0;
		unsigned int smpl = m_Sample;
		for (unsigned short i = 0; i < c_Average; ++i) {
			accum += m_PerfData[counter][smpl];
			smpl--;
			if (smpl == 0) { smpl = c_MaxSamples; }
		}
		return accum / c_Average;
	}
}