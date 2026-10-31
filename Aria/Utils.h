#ifndef _Aria_Utils_h_
#define _Aria_Utils_h_

template <class F>
auto Retry(F func, int tries = 3, int delay_ms = 1000, double backoff = 2.0, double jitter = 0.1, AriaLogger& logger = GetAriaLogger("utils"))
-> decltype(func())
{
	int m_tries = tries;
	int m_delay = delay_ms;
	
	while (m_tries > 1) {
		try {
			return func();
		}
		catch (const Exc& e) {
			double j = (Random(2000) - 1000) / 1000.0 * jitter;
			int wait = int(m_delay * (1.0 + j));
			
			logger.Warning(Format("Retrying in %d ms. Attempts remaining: %d. Error: %s", wait, m_tries - 1, e));
			
			Sleep(wait);
			m_tries--;
			m_delay = int(m_delay * backoff);
		}
	}
	
	return func(); // Final attempt
}

#endif