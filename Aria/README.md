# Aria

**The Intelligent Navigator for the Modern Web.**

Aria is the AI-driven orchestration layer for web interaction. Born from the need to alleviate cognitive fatigue in the digital age, Aria transforms the web browser from a passive viewer into an active, intelligent agent. It is the third pillar in our ecosystem of efficiency:

*   **Maestro:** The "Swiss Army Knife" of project management and cross-language build systems.
*   **Concert:** The "Swiss Army Knife" for the modern entrepreneur and strategic operative.
*   **Aria:** The "Swiss Army Knife" for the web—automating the mundane to reclaim your most valuable asset: time.

This version of Aria is implemented using the **Ultimate++ (U++)** framework in C++, providing high performance and seamless integration with our efficiency ecosystem.

---

## 👁️ The Vision

In an era of information overload and "dark patterns" designed to capture your attention, Aria acts as your digital proxy. Whether you are a casual consumer tired of endless clicking or an enterprise professional seeking to streamline complex web-based workflows, Aria provides a seamless interface between human intent and web execution.

We aren't just building a tool; we are building the future of **Autonomous Web Navigation (AWN)**. Aria leverages advanced AI to understand web structures, interpret user goals, and execute multi-step processes with precision.

---

## 🚀 Key Features & Use Cases

### For the Casual Consumer: "Digital Zen"
*   **Cognitive Load Reduction:** Tired of reading 20 reviews for a toaster? Tell Aria to "Find the most reliable toaster under $50 with a 2-year warranty" and watch it navigate, compare, and present the winner.
*   **One-Click Fulfillment:** Automate tedious forms, checkout processes, and booking flows.
*   **Signal Over Noise:** Aria filters the clutter, extracting only the data that matters to your specific query.

### For the Professional: "The Strategic Advantage"
*   **Market Intelligence:** Automatically track competitor pricing, sentiment shifts, and news cycles without manual refreshing.
*   **Data Orchestration:** Seamlessly bridge the gap between web-based SaaS tools that lack native integrations.
*   **Research Acceleration:** Rapidly synthesize information from disparate sources into structured reports.

### For the Enterprise: "Next-Gen RPA"
Aria is architected with a "Pro-Product" mindset. While it solves individual pain points today, it is designed to scale into a robust enterprise solution for:
*   **Hyper-Automation:** Integrating with legacy web portals that lack APIs.
*   **Compliance Monitoring:** Auditing web-facing assets and content at scale.
*   **Operational Excellence:** Reducing human error in high-volume web tasks.

---

## 🛠️ Prerequisites & Installation

Aria utilizes industry-standard webdrivers to securely and effectively pilot your browser.

### 1. System Package Manager
Ensure you have a package manager installed for your operating system:
*   **Windows:** [Chocolatey](https://chocolatey.org/)
*   **macOS:** [Homebrew](https://brew.sh/)
*   **Linux:** Your distribution's native manager (apt, dnf, pacman).

### 2. Webdrivers
Aria requires Selenium webdrivers to interface with your browsers. 

**On Windows (Run as Administrator):**
```powershell
choco install selenium-all-drivers
```

**On Gentoo Linux:**
```bash
sudo emerge --noreplace net-misc/geckodriver www-apps/chromedriver-bin
```

### 3. Starting Webdrivers
Aria starts the appropriate WebDriver server automatically when needed.
To start Firefox's server manually, use:

**Firefox (geckodriver):**
```bash
geckodriver --port=4444
```

Aria starts `geckodriver` automatically when the Suno command needs a new
session, provided that `geckodriver` is installed and available in `PATH`.

### 4. Build from Source (U++)
This project is built using the `umk` utility or TheIDE.
```bash
bin/build Aria
```
The Linux build is written to `bin/CLANG/Aria`.

To capture the currently active Suno playlist tab:
```bash
bin/Aria site suno ~/Musiikki/
```
The command only uses the existing Aria Firefox session. Start it first with
`bin/Aria open <url>`; `site suno` never opens a second Firefox window.
To click and save every track in the active playlist, use:
```bash
bin/Aria site suno all ~/Musiikki/
```
To process all playlists whose name starts with `Steve Chill`:
```bash
bin/Aria site suno playlists "Steve Chill" ~/Musiikki/
```
To retry only tracks whose saved lyrics are empty:
```bash
bin/Aria site suno retry-lyrics ~/Musiikki/
```

---

## 📚 Documentation
- [AGENTS.md](AGENTS.md): Guide for AI agents working on this codebase.
- [plan/](plan/): Detailed development roadmap and task tracking.

---

## 📈 The Roadmap: From Tool to Ecosystem

We are currently in the early stages of development. Our trajectory includes:
1.  **Phase I: Foundation** - Core AI-to-Webdriver mapping and basic intent recognition.
2.  **Phase II: Mastery** - Complex multi-page workflow execution and "Human-in-the-loop" feedback systems.
3.  **Phase III: Enterprise Integration** - Secure credential vaulting, multi-agent collaboration, and API-first architecture for B2B deployments.

---

*Aria is part of the future of autonomous digital living. Join us as we redefine how the world interacts with the web.*
