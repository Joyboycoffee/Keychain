const fs = require('fs');

// 1. FULL WIRING SCHEMATIC SVG
const wiringSvg = `
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1200 850" width="100%" height="100%" style="background-color: #0b0f19; font-family: 'Segoe UI', Roboto, sans-serif;">
  <defs>
    <filter id="glow" x="-20%" y="-20%" width="140%" height="140%">
      <feGaussianBlur stdDeviation="4" result="blur" />
      <feComposite in="SourceGraphic" in2="blur" operator="over" />
    </filter>
    <linearGradient id="gradEsp" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#1e293b"/>
      <stop offset="100%" stop-color="#0f172a"/>
    </linearGradient>
    <linearGradient id="gradDisplay" x1="0%" y1="0%" x2="0%" y2="100%">
      <stop offset="0%" stop-color="#1e1b4b"/>
      <stop offset="100%" stop-color="#0f172a"/>
    </linearGradient>
    <linearGradient id="gradTp" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#0369a1"/>
      <stop offset="100%" stop-color="#075985"/>
    </linearGradient>
    <linearGradient id="gradBat" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#eab308"/>
      <stop offset="100%" stop-color="#ca8a04"/>
    </linearGradient>
    <linearGradient id="gradTouch" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#b91c1c"/>
      <stop offset="100%" stop-color="#991b1b"/>
    </linearGradient>
  </defs>

  <!-- Title & Header -->
  <text x="600" y="45" font-size="28" font-weight="bold" fill="#00f0ff" text-anchor="middle" letter-spacing="2" filter="url(#glow)">⚡ COMPLETE ESP32-C3 CYBER KEYCHAIN WIRING BLUEPRINT</text>
  <text x="600" y="75" font-size="14" fill="#94a3b8" text-anchor="middle">Minimalist Design • Single 4.7kΩ Resistor Mod • Full Signal &amp; Power Routing</text>

  <!-- ==================== MODULE: 1.3" IPS DISPLAY ==================== -->
  <g transform="translate(80, 110)">
    <rect x="0" y="0" width="340" height="280" rx="12" fill="url(#gradDisplay)" stroke="#38bdf8" stroke-width="2"/>
    <text x="170" y="30" font-size="16" font-weight="bold" fill="#38bdf8" text-anchor="middle">1.3" IPS LCD (ST7789 240x240)</text>
    
    <!-- Screen Glass Area -->
    <rect x="30" y="45" width="200" height="200" rx="6" fill="#000" stroke="#0284c7" stroke-width="1.5"/>
    <text x="130" y="135" font-size="14" fill="#00f0ff" text-anchor="middle" opacity="0.8">IPS Screen Face</text>
    <text x="130" y="155" font-size="12" fill="#38bdf8" text-anchor="middle" opacity="0.6">240 x 240 RGB</text>

    <!-- Display Pin Header on Right -->
    <!-- Pins: GND, VCC, SCL, SDA, RES, DC, BLK -->
    <g transform="translate(250, 45)">
      <rect x="0" y="0" width="70" height="210" rx="4" fill="#1e293b" stroke="#475569"/>
      
      <!-- Pin 1: GND -->
      <circle cx="15" cy="18" r="6" fill="#000" stroke="#94a3b8" stroke-width="1.5"/>
      <text x="32" y="23" font-size="12" font-weight="bold" fill="#fff">GND</text>

      <!-- Pin 2: VCC -->
      <circle cx="15" cy="46" r="6" fill="#ef4444" stroke="#94a3b8" stroke-width="1.5"/>
      <text x="32" y="51" font-size="12" font-weight="bold" fill="#fff">VCC</text>

      <!-- Pin 3: SCL -->
      <circle cx="15" cy="74" r="6" fill="#eab308" stroke="#94a3b8" stroke-width="1.5"/>
      <text x="32" y="79" font-size="12" font-weight="bold" fill="#fff">SCL</text>

      <!-- Pin 4: SDA -->
      <circle cx="15" cy="102" r="6" fill="#22c55e" stroke="#94a3b8" stroke-width="1.5"/>
      <text x="32" y="107" font-size="12" font-weight="bold" fill="#fff">SDA</text>

      <!-- Pin 5: RES -->
      <circle cx="15" cy="130" r="6" fill="#3b82f6" stroke="#94a3b8" stroke-width="1.5"/>
      <text x="32" y="135" font-size="12" font-weight="bold" fill="#fff">RES</text>

      <!-- Pin 6: DC -->
      <circle cx="15" cy="158" r="6" fill="#a855f7" stroke="#94a3b8" stroke-width="1.5"/>
      <text x="32" y="163" font-size="12" font-weight="bold" fill="#fff">DC</text>

      <!-- Pin 7: BLK -->
      <circle cx="15" cy="186" r="6" fill="#f8fafc" stroke="#94a3b8" stroke-width="1.5"/>
      <text x="32" y="191" font-size="12" font-weight="bold" fill="#fff">BLK</text>
    </g>
  </g>

  <!-- ==================== MODULE: ESP32-C3 SuperMini ==================== -->
  <g transform="translate(560, 110)">
    <rect x="0" y="0" width="300" height="380" rx="14" fill="url(#gradEsp)" stroke="#6366f1" stroke-width="2.5"/>
    <text x="150" y="32" font-size="18" font-weight="bold" fill="#a5b4fc" text-anchor="middle">ESP32-C3 SuperMini</text>
    <text x="150" y="50" font-size="11" fill="#64748b" text-anchor="middle">RISC-V 32-bit • BLE 5.0 • WiFi</text>

    <!-- USB-C Port Mockup at Top -->
    <rect x="105" y="-12" width="90" height="24" rx="6" fill="#334155" stroke="#94a3b8" stroke-width="2"/>
    <text x="150" y="5" font-size="10" font-weight="bold" fill="#e2e8f0" text-anchor="middle">USB-C PORT</text>

    <!-- ESP32 Chip & Antenna -->
    <rect x="80" y="75" width="140" height="130" rx="6" fill="#090d16" stroke="#4f46e5" stroke-width="1.5"/>
    <rect x="95" y="90" width="110" height="90" rx="4" fill="#1e1b4b" stroke="#6366f1" stroke-width="1"/>
    <text x="150" y="135" font-size="13" font-weight="bold" fill="#c7d2fe" text-anchor="middle">ESP32-C3</text>
    <text x="150" y="152" font-size="10" fill="#818cf8" text-anchor="middle">RISC-V SoC</text>

    <!-- Left Header Pins (Pins 1 to 8) -->
    <!-- Pin List: 3V3, GND, G0, G1, G2, G3, G4, G5 -->
    <g transform="translate(15, 80)">
      <!-- 3V3 -->
      <circle cx="10" cy="15" r="7" fill="#ef4444" stroke="#fff" stroke-width="1.5"/>
      <text x="26" y="20" font-size="12" font-weight="bold" fill="#fca5a5">3V3</text>

      <!-- GND -->
      <circle cx="10" cy="48" r="7" fill="#000" stroke="#fff" stroke-width="1.5"/>
      <text x="26" y="53" font-size="12" font-weight="bold" fill="#cbd5e1">GND</text>

      <!-- G0 -->
      <circle cx="10" cy="81" r="7" fill="#334155" stroke="#94a3b8" stroke-width="1.5"/>
      <text x="26" y="86" font-size="12" fill="#94a3b8">G0</text>

      <!-- G1 (Touch) -->
      <circle cx="10" cy="114" r="7" fill="#ea580c" stroke="#fff" stroke-width="1.5"/>
      <text x="26" y="119" font-size="12" font-weight="bold" fill="#fdba74">G1 (Touch)</text>

      <!-- G2 (BLK) -->
      <circle cx="10" cy="147" r="7" fill="#f8fafc" stroke="#64748b" stroke-width="1.5"/>
      <text x="26" y="152" font-size="12" font-weight="bold" fill="#f8fafc">G2 (BLK)</text>

      <!-- G3 (DC) -->
      <circle cx="10" cy="180" r="7" fill="#a855f7" stroke="#fff" stroke-width="1.5"/>
      <text x="26" y="185" font-size="12" font-weight="bold" fill="#d8b4fe">G3 (DC)</text>

      <!-- G4 -->
      <circle cx="10" cy="213" r="7" fill="#334155" stroke="#94a3b8" stroke-width="1.5"/>
      <text x="26" y="218" font-size="12" fill="#94a3b8">G4</text>

      <!-- G5 (RES) -->
      <circle cx="10" cy="246" r="7" fill="#3b82f6" stroke="#fff" stroke-width="1.5"/>
      <text x="26" y="251" font-size="12" font-weight="bold" fill="#93c5fd">G5 (RES)</text>
    </g>

    <!-- Right Header Pins (Pins 9 to 16) -->
    <!-- Pin List: 5V, GND, G6 (SCL), G7, G8(SCL), G9, G10(SDA), G20/21 -->
    <g transform="translate(285, 80)">
      <!-- 5V -->
      <circle cx="-10" cy="15" r="7" fill="#dc2626" stroke="#fff" stroke-width="1.5"/>
      <text x="-30" y="20" font-size="12" font-weight="bold" fill="#f87171" text-anchor="end">5V (Power)</text>

      <!-- GND -->
      <circle cx="-10" cy="48" r="7" fill="#000" stroke="#fff" stroke-width="1.5"/>
      <text x="-30" y="53" font-size="12" font-weight="bold" fill="#cbd5e1" text-anchor="end">GND</text>

      <!-- G6 (SCL) -->
      <circle cx="-10" cy="81" r="7" fill="#334155" stroke="#94a3b8" stroke-width="1.5"/>
      <text x="-30" y="86" font-size="12" fill="#94a3b8" text-anchor="end">G6 (SCL)</text>

      <!-- G7 -->
      <circle cx="-10" cy="114" r="7" fill="#334155" stroke="#94a3b8" stroke-width="1.5"/>
      <text x="-30" y="119" font-size="12" fill="#94a3b8" text-anchor="end">G7</text>

      <!-- G8 (LED) -->
      <circle cx="-10" cy="147" r="7" fill="#eab308" stroke="#fff" stroke-width="1.5"/>
      <text x="-30" y="152" font-size="12" font-weight="bold" fill="#fde047" text-anchor="end">G8 (LED)</text>

      <!-- G9 -->
      <circle cx="-10" cy="180" r="7" fill="#334155" stroke="#94a3b8" stroke-width="1.5"/>
      <text x="-30" y="185" font-size="12" fill="#94a3b8" text-anchor="end">G9</text>

      <!-- G10 (SDA) -->
      <circle cx="-10" cy="213" r="7" fill="#22c55e" stroke="#fff" stroke-width="1.5"/>
      <text x="-30" y="218" font-size="12" font-weight="bold" fill="#86efac" text-anchor="end">G10 (SDA)</text>

      <!-- G20 -->
      <circle cx="-10" cy="246" r="7" fill="#334155" stroke="#94a3b8" stroke-width="1.5"/>
      <text x="-30" y="251" font-size="12" fill="#94a3b8" text-anchor="end">G20</text>
    </g>
  </g>

  <!-- ==================== MODULE: TTP223 TOUCH SENSOR ==================== -->
  <g transform="translate(940, 110)">
    <rect x="0" y="0" width="180" height="150" rx="8" fill="url(#gradTouch)" stroke="#f87171" stroke-width="2"/>
    <text x="90" y="25" font-size="14" font-weight="bold" fill="#fff" text-anchor="middle">TTP223 Touch Pad</text>

    <!-- Circular Touch Zone -->
    <circle cx="90" cy="75" r="28" fill="#7f1d1d" stroke="#fca5a5" stroke-dasharray="3,3" stroke-width="2"/>
    <text x="90" y="79" font-size="11" font-weight="bold" fill="#fecaca" text-anchor="middle">TOUCH</text>

    <!-- 3 Header Pins -->
    <g transform="translate(30, 120)">
      <circle cx="15" cy="10" r="6" fill="#ef4444" stroke="#fff" stroke-width="1.5"/>
      <text x="15" y="-3" font-size="10" font-weight="bold" fill="#fff" text-anchor="middle">VCC</text>

      <circle cx="60" cy="10" r="6" fill="#000" stroke="#fff" stroke-width="1.5"/>
      <text x="60" y="-3" font-size="10" font-weight="bold" fill="#fff" text-anchor="middle">GND</text>

      <circle cx="105" cy="10" r="6" fill="#ea580c" stroke="#fff" stroke-width="1.5"/>
      <text x="105" y="-3" font-size="10" font-weight="bold" fill="#fff" text-anchor="middle">I/O</text>
    </g>
  </g>

  <!-- ==================== MODULE: TP4056 CHARGER ==================== -->
  <g transform="translate(80, 480)">
    <rect x="0" y="0" width="340" height="260" rx="10" fill="url(#gradTp)" stroke="#38bdf8" stroke-width="2"/>
    <text x="170" y="28" font-size="16" font-weight="bold" fill="#fff" text-anchor="middle">TP4056 LiPo Charger Module</text>
    
    <!-- Chip details & Resistor Mod Callout -->
    <rect x="110" y="80" width="120" height="80" rx="4" fill="#082f49" stroke="#0284c7" stroke-width="1.5"/>
    <text x="170" y="115" font-size="12" font-weight="bold" fill="#38bdf8" text-anchor="middle">TP4056 IC</text>
    <rect x="135" y="128" width="70" height="22" rx="3" fill="#eab308"/>
    <text x="170" y="143" font-size="10" font-weight="bold" fill="#000" text-anchor="middle">MOD: 4.7kΩ (250mA)</text>

    <!-- IN+ / IN- on Left -->
    <circle cx="25" cy="65" r="7" fill="#dc2626" stroke="#fff" stroke-width="1.5"/>
    <text x="40" y="70" font-size="12" font-weight="bold" fill="#fff">IN+</text>

    <circle cx="25" cy="195" r="7" fill="#000" stroke="#fff" stroke-width="1.5"/>
    <text x="40" y="200" font-size="12" font-weight="bold" fill="#fff">IN-</text>

    <!-- BAT+ / BAT- on Right -->
    <circle cx="315" cy="65" r="7" fill="#dc2626" stroke="#fff" stroke-width="1.5"/>
    <text x="300" y="70" font-size="12" font-weight="bold" fill="#fff" text-anchor="end">BAT+</text>

    <circle cx="315" cy="195" r="7" fill="#000" stroke="#fff" stroke-width="1.5"/>
    <text x="300" y="200" font-size="12" font-weight="bold" fill="#fff" text-anchor="end">BAT-</text>
  </g>

  <!-- ==================== MODULE: SPDT SLIDE SWITCH ==================== -->
  <g transform="translate(520, 560)">
    <rect x="0" y="0" width="130" height="90" rx="8" fill="#334155" stroke="#94a3b8" stroke-width="2"/>
    <text x="65" y="25" font-size="12" font-weight="bold" fill="#f8fafc" text-anchor="middle">Slide Switch</text>
    
    <!-- Switch Lever -->
    <rect x="45" y="32" width="40" height="18" rx="3" fill="#0f172a" stroke="#cbd5e1" stroke-width="1.5"/>
    <circle cx="55" cy="41" r="5" fill="#38bdf8"/>

    <!-- 3 Terminals -->
    <circle cx="25" cy="72" r="6" fill="#94a3b8" stroke="#fff" stroke-width="1.5"/>
    <text x="25" y="87" font-size="9" fill="#cbd5e1" text-anchor="middle">T1</text>

    <circle cx="65" cy="72" r="6" fill="#e2e8f0" stroke="#fff" stroke-width="1.5"/>
    <text x="65" y="87" font-size="9" font-weight="bold" fill="#fff" text-anchor="middle">COM</text>

    <circle cx="105" cy="72" r="6" fill="#94a3b8" stroke="#fff" stroke-width="1.5"/>
    <text x="105" y="87" font-size="9" fill="#cbd5e1" text-anchor="middle">T2</text>
  </g>

  <!-- ==================== MODULE: 3.7V 200mAh LiPo ==================== -->
  <g transform="translate(740, 540)">
    <rect x="0" y="0" width="280" height="170" rx="10" fill="url(#gradBat)" stroke="#fef08a" stroke-width="2"/>
    <text x="140" y="32" font-size="16" font-weight="bold" fill="#000" text-anchor="middle">3.7V 200mAh LiPo (502025)</text>
    <text x="140" y="52" font-size="12" fill="#422006" text-anchor="middle">0.74Wh Rechargeable Pouch Cell</text>

    <rect x="30" y="70" width="220" height="70" rx="6" fill="#713f12" opacity="0.2"/>
    <text x="140" y="110" font-size="14" font-weight="bold" fill="#000" text-anchor="middle">⚡ LITHIUM POLYMER</text>

    <!-- Battery Wire Leads -->
    <circle cx="30" cy="150" r="7" fill="#dc2626" stroke="#fff" stroke-width="1.5"/>
    <text x="45" y="155" font-size="12" font-weight="bold" fill="#000">RED (+)</text>

    <circle cx="250" cy="150" r="7" fill="#000" stroke="#fff" stroke-width="1.5"/>
    <text x="235" y="155" font-size="12" font-weight="bold" fill="#000" text-anchor="end">BLACK (-)</text>
  </g>

  <!-- ==================== WIRING TRACES & CONNECTIONS ==================== -->

  <!-- 1. SPI CLOCK (SCL): Display SCL -> ESP32 G8 -->
  <path d="M 335 184 L 460 184 L 460 70 L 890 70 L 890 227 L 835 227" fill="none" stroke="#eab308" stroke-width="3"/>

  <!-- 2. SPI DATA (SDA): Display SDA -> ESP32 G10 -->
  <path d="M 335 212 L 475 212 L 475 85 L 905 85 L 905 293 L 835 293" fill="none" stroke="#22c55e" stroke-width="3"/>

  <!-- 3. RESET (RES): Display RES -> ESP32 G5 -->
  <path d="M 335 240 L 510 240 L 510 326 L 585 326" fill="none" stroke="#3b82f6" stroke-width="3"/>

  <!-- 4. DATA/COMMAND (DC): Display DC -> ESP32 G3 -->
  <path d="M 335 268 L 490 268 L 490 260 L 585 260" fill="none" stroke="#a855f7" stroke-width="3"/>

  <!-- 5. BACKLIGHT (BLK): Display BLK -> ESP32 G2 -->
  <path d="M 335 296 L 470 296 L 470 227 L 585 227" fill="none" stroke="#f8fafc" stroke-width="2.5" stroke-dasharray="6,3"/>

  <!-- 6. 3.3V POWER RAILS: ESP32 3V3 -> Display VCC & Touch VCC -->
  <path d="M 585 95 L 450 95 L 450 156 L 335 156" fill="none" stroke="#ef4444" stroke-width="3"/>
  <path d="M 585 95 L 985 95 L 985 220" fill="none" stroke="#ef4444" stroke-width="3"/>

  <!-- 7. TOUCH SIGNAL: Touch I/O -> ESP32 G1 -->
  <path d="M 1075 230 L 1075 350 L 525 350 L 525 194 L 585 194" fill="none" stroke="#ea580c" stroke-width="3"/>

  <!-- 8. TOUCH GROUND: Touch GND -> ESP32 GND -->
  <path d="M 1030 230 L 1030 410 L 550 410 L 550 128 L 585 128" fill="none" stroke="#000" stroke-width="3.5"/>
  <path d="M 1030 230 L 1030 410 L 550 410 L 550 128 L 585 128" fill="none" stroke="#64748b" stroke-width="1.5"/>

  <!-- 9. DISPLAY GROUND: Display GND -> ESP32 GND -->
  <path d="M 335 128 L 585 128" fill="none" stroke="#000" stroke-width="4"/>
  <path d="M 335 128 L 585 128" fill="none" stroke="#64748b" stroke-width="1.5"/>

  <!-- 10. 5V CHARGE SINK: ESP32 5V Pin -> TP4056 IN+ -->
  <path d="M 835 95 L 870 95 L 870 450 L 50 450 L 50 545 L 105 545" fill="none" stroke="#dc2626" stroke-width="3.5"/>

  <!-- 11. CHARGER GROUND: ESP32 GND Pin -> TP4056 IN- -->
  <path d="M 835 128 L 860 128 L 860 465 L 40 465 L 40 675 L 105 675" fill="none" stroke="#000" stroke-width="4"/>
  <path d="M 835 128 L 860 128 L 860 465 L 40 465 L 40 675 L 105 675" fill="none" stroke="#64748b" stroke-width="1.5"/>

  <!-- 12. BATTERY NEGATIVE: Battery (-) -> TP4056 BAT- & ESP32 GND -->
  <path d="M 990 690 L 990 770 L 405 770 L 405 675 L 395 675" fill="none" stroke="#000" stroke-width="4"/>
  <path d="M 990 690 L 990 770 L 405 770 L 405 675 L 395 675" fill="none" stroke="#64748b" stroke-width="1.5"/>

  <!-- 13. BATTERY POSITIVE & SWITCH ROUTING:
       Battery (+) -> Switch COM (Pin 2)
       Switch T1 (Pin 1) -> TP4056 BAT+
       Switch COM (Pin 2) -> ESP32 5V Pin (Power Feed)
  -->
  <path d="M 770 690 L 770 730 L 585 730 L 585 632" fill="none" stroke="#dc2626" stroke-width="3.5"/>
  <path d="M 545 632 L 545 545 L 395 545" fill="none" stroke="#dc2626" stroke-width="3.5"/>
  <path d="M 585 632 L 680 632 L 680 430 L 835 430 L 835 105" fill="none" stroke="#dc2626" stroke-width="3.5" stroke-dasharray="5,3"/>

  <!-- Junction Dots -->
  <circle cx="585" cy="128" r="5" fill="#38bdf8"/>
  <circle cx="585" cy="95" r="5" fill="#ef4444"/>
  <circle cx="835" cy="95" r="5" fill="#dc2626"/>
  <circle cx="835" cy="128" r="5" fill="#38bdf8"/>
  <circle cx="585" cy="632" r="5" fill="#dc2626"/>

  <!-- Legend Box at Bottom Right -->
  <g transform="translate(860, 480)">
    <rect x="0" y="0" width="310" height="240" rx="8" fill="#0f172a" stroke="#334155" stroke-width="1.5"/>
    <text x="155" y="24" font-size="13" font-weight="bold" fill="#f8fafc" text-anchor="middle">WIRE COLOR LEGEND</text>
    
    <circle cx="20" cy="50" r="6" fill="#eab308"/>
    <text x="35" y="54" font-size="11" fill="#f8fafc">SPI Clock (SCL (GPIO 6))</text>

    <circle cx="20" cy="76" r="6" fill="#22c55e"/>
    <text x="35" y="80" font-size="11" fill="#f8fafc">SPI Data (SDA / MOSI)</text>

    <circle cx="20" cy="102" r="6" fill="#3b82f6"/>
    <text x="35" y="106" font-size="11" fill="#f8fafc">Screen Reset (RES)</text>

    <circle cx="20" cy="128" r="6" fill="#a855f7"/>
    <text x="35" y="132" font-size="11" fill="#f8fafc">Data/Command (DC)</text>

    <circle cx="20" cy="154" r="6" fill="#f8fafc"/>
    <text x="35" y="158" font-size="11" fill="#f8fafc">Backlight PWM (BLK)</text>

    <circle cx="20" cy="180" r="6" fill="#ea580c"/>
    <text x="35" y="184" font-size="11" fill="#f8fafc">Touch Wake Signal (I/O)</text>

    <circle cx="20" cy="206" r="6" fill="#dc2626"/>
    <text x="35" y="210" font-size="11" fill="#f8fafc">Power Lines (3.3V / 5V / BAT)</text>
  </g>
</svg>
`;

// 2. TP4056 RESISTOR MOD CLOSE-UP SVG
const tpModSvg = `
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 900 500" width="100%" height="100%" style="background-color: #0b0f19; font-family: 'Segoe UI', Roboto, sans-serif;">
  <defs>
    <filter id="glow" x="-20%" y="-20%" width="140%" height="140%">
      <feGaussianBlur stdDeviation="3" result="blur" />
      <feComposite in="SourceGraphic" in2="blur" operator="over" />
    </filter>
  </defs>

  <text x="450" y="40" font-size="22" font-weight="bold" fill="#00f0ff" text-anchor="middle" filter="url(#glow)">TP4056 CURRENT MODIFICATION FOR 200mAh BATTERY</text>
  <text x="450" y="65" font-size="13" fill="#94a3b8" text-anchor="middle">Step-by-step: Swap 1.2kΩ (122) SMD Resistor with 4.7kΩ for Safe 250mA Charging</text>

  <!-- Board Outline -->
  <g transform="translate(80, 100)">
    <rect x="0" y="0" width="400" height="320" rx="12" fill="#0369a1" stroke="#38bdf8" stroke-width="2.5"/>
    <text x="200" y="35" font-size="16" font-weight="bold" fill="#fff" text-anchor="middle">TP4056 CHARGING BOARD</text>

    <!-- USB Socket Mockup -->
    <rect x="-15" y="110" width="50" height="90" rx="4" fill="#64748b" stroke="#cbd5e1" stroke-width="2"/>
    <text x="10" y="160" font-size="10" font-weight="bold" fill="#000" transform="rotate(-90 10 160)" text-anchor="middle">USB-IN</text>

    <!-- TP4056 SOP-8 Chip -->
    <rect x="150" y="110" width="120" height="100" rx="4" fill="#0f172a" stroke="#0284c7" stroke-width="1.5"/>
    <circle cx="165" cy="125" r="4" fill="#38bdf8"/> <!-- Pin 1 dot -->
    <text x="210" y="155" font-size="14" font-weight="bold" fill="#38bdf8" text-anchor="middle">TP4056</text>
    <text x="210" y="175" font-size="10" fill="#94a3b8" text-anchor="middle">SOP-8</text>

    <!-- Resistor Rprog (R3 / 122) Location -->
    <g transform="translate(180, 235)">
      <rect x="0" y="0" width="55" height="28" rx="3" fill="#ef4444" stroke="#fca5a5" stroke-width="2" filter="url(#glow)"/>
      <text x="27" y="18" font-size="12" font-weight="bold" fill="#fff" text-anchor="middle">122</text>
      <text x="27" y="42" font-size="11" font-weight="bold" fill="#fca5a5" text-anchor="middle">Rprog (Pin 2)</text>
    </g>

    <!-- LEDs -->
    <circle cx="100" cy="70" r="8" fill="#dc2626" stroke="#fff" stroke-width="1"/>
    <text x="100" y="95" font-size="10" fill="#fff" text-anchor="middle">CHRG (Red)</text>

    <circle cx="280" cy="70" r="8" fill="#2563eb" stroke="#fff" stroke-width="1"/>
    <text x="280" y="95" font-size="10" fill="#fff" text-anchor="middle">STBY (Blue)</text>
  </g>

  <!-- Explanation & Math Box on Right -->
  <g transform="translate(520, 100)">
    <rect x="0" y="0" width="320" height="320" rx="10" fill="#1e293b" stroke="#475569" stroke-width="1.5"/>
    
    <text x="20" y="35" font-size="15" font-weight="bold" fill="#eab308">📐 The Charging Formula:</text>
    <rect x="20" y="50" width="280" height="40" rx="4" fill="#0f172a"/>
    <text x="160" y="75" font-size="14" font-weight="bold" fill="#00f0ff" text-anchor="middle">I_charge = 1200 / R_prog</text>

    <text x="20" y="120" font-size="13" fill="#f87171">❌ Stock Resistor (122 = 1.2kΩ):</text>
    <text x="35" y="140" font-size="13" fill="#cbd5e1">I = 1200 / 1200 = <tspan font-weight="bold" fill="#f87171">1000mA (1A)</tspan></text>
    <text x="35" y="158" font-size="11" fill="#94a3b8">Too high! Will puff up a 200mAh LiPo.</text>

    <text x="20" y="195" font-size="13" fill="#4ade80">✅ Swapped Resistor (4.7kΩ):</text>
    <text x="35" y="215" font-size="13" fill="#cbd5e1">I = 1200 / 4700 = <tspan font-weight="bold" fill="#4ade80">~255mA (0.25A)</tspan></text>
    <text x="35" y="233" font-size="11" fill="#94a3b8">Safe, gentle charging in ~50 minutes!</text>

    <rect x="20" y="255" width="280" height="45" rx="4" fill="#0284c7" opacity="0.2"/>
    <text x="160" y="275" font-size="11" font-weight="bold" fill="#38bdf8" text-anchor="middle">Tip: Heat with soldering iron tip,</text>
    <text x="160" y="290" font-size="11" fill="#bae6fd" text-anchor="middle">flick off '122' &amp; solder 4.7kΩ across pads!</text>
  </g>
</svg>
`;

// 3. PHYSICAL SANDWICH / PERFBOARD ASSEMBLY SVG
const sandwichSvg = `
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1000 600" width="100%" height="100%" style="background-color: #0b0f19; font-family: 'Segoe UI', Roboto, sans-serif;">
  <defs>
    <filter id="glow" x="-20%" y="-20%" width="140%" height="140%">
      <feGaussianBlur stdDeviation="3" result="blur" />
      <feComposite in="SourceGraphic" in2="blur" operator="over" />
    </filter>
  </defs>

  <text x="500" y="40" font-size="22" font-weight="bold" fill="#00f0ff" text-anchor="middle" filter="url(#glow)">PHYSICAL PERFBOARD SANDWICH ASSEMBLY</text>
  <text x="500" y="65" font-size="13" fill="#94a3b8" text-anchor="middle">Exploded View • 35mm x 55mm Form Factor • Rugged Backpack Construction</text>

  <!-- Layer 1: Front Protective Acrylic -->
  <g transform="translate(60, 110)">
    <rect x="0" y="0" width="140" height="240" rx="8" fill="#38bdf8" opacity="0.25" stroke="#38bdf8" stroke-width="2"/>
    <circle cx="20" cy="20" r="8" fill="none" stroke="#fff" stroke-width="2"/>
    <text x="70" y="125" font-size="13" font-weight="bold" fill="#fff" text-anchor="middle">Layer 1: Lens</text>
    <text x="70" y="145" font-size="10" fill="#cbd5e1" text-anchor="middle">1mm Clear Acrylic</text>
    <text x="70" y="160" font-size="10" fill="#94a3b8" text-anchor="middle">(Scratch Guard)</text>
  </g>
  <text x="220" y="235" font-size="22" fill="#64748b" text-anchor="middle">➔</text>

  <!-- Layer 2: 1.3" IPS LCD -->
  <g transform="translate(240, 110)">
    <rect x="0" y="0" width="140" height="240" rx="8" fill="#1e1b4b" stroke="#6366f1" stroke-width="2"/>
    <rect x="15" y="30" width="110" height="130" rx="4" fill="#000" stroke="#818cf8"/>
    <text x="70" y="100" font-size="11" font-weight="bold" fill="#00f0ff" text-anchor="middle">1.3" IPS Screen</text>
    <text x="70" y="195" font-size="13" font-weight="bold" fill="#fff" text-anchor="middle">Layer 2: Display</text>
    <text x="70" y="215" font-size="10" fill="#c7d2fe" text-anchor="middle">Header faces down</text>
  </g>
  <text x="400" y="235" font-size="22" fill="#64748b" text-anchor="middle">➔</text>

  <!-- Layer 3: Green Perfboard Backbone -->
  <g transform="translate(420, 110)">
    <rect x="0" y="0" width="150" height="240" rx="8" fill="#064e3b" stroke="#10b981" stroke-width="2.5"/>
    <!-- Dot matrix pattern -->
    <g fill="#047857">
      <circle cx="25" cy="50" r="2.5"/><circle cx="50" cy="50" r="2.5"/><circle cx="75" cy="50" r="2.5"/><circle cx="100" cy="50" r="2.5"/><circle cx="125" cy="50" r="2.5"/>
      <circle cx="25" cy="75" r="2.5"/><circle cx="50" cy="75" r="2.5"/><circle cx="75" cy="75" r="2.5"/><circle cx="100" cy="75" r="2.5"/><circle cx="125" cy="75" r="2.5"/>
      <circle cx="25" cy="100" r="2.5"/><circle cx="50" cy="100" r="2.5"/><circle cx="75" cy="100" r="2.5"/><circle cx="100" cy="100" r="2.5"/><circle cx="125" cy="100" r="2.5"/>
    </g>
    <circle cx="20" cy="20" r="8" fill="#090d16" stroke="#fff" stroke-width="2"/>
    <text x="75" y="150" font-size="13" font-weight="bold" fill="#a7f3d0" text-anchor="middle">Layer 3: Perfboard</text>
    <text x="75" y="170" font-size="10" fill="#6ee7b7" text-anchor="middle">Central Backbone</text>
    <text x="75" y="185" font-size="9" fill="#94a3b8" text-anchor="middle">(Insulated with lacquer)</text>
  </g>
  <text x="590" y="235" font-size="22" fill="#64748b" text-anchor="middle">➔</text>

  <!-- Layer 4: Electronics (ESP32 + TP4056 + Touch) -->
  <g transform="translate(610, 110)">
    <rect x="0" y="0" width="150" height="240" rx="8" fill="#0f172a" stroke="#64748b" stroke-width="2"/>
    <!-- ESP32 -->
    <rect x="15" y="20" width="120" height="90" rx="4" fill="#1e293b" stroke="#6366f1"/>
    <text x="75" y="65" font-size="10" font-weight="bold" fill="#a5b4fc" text-anchor="middle">ESP32-C3 SuperMini</text>
    <!-- TP4056 -->
    <rect x="15" y="120" width="75" height="70" rx="4" fill="#0369a1" stroke="#38bdf8"/>
    <text x="52" y="158" font-size="9" font-weight="bold" fill="#fff" text-anchor="middle">TP4056</text>
    <!-- Touch -->
    <rect x="95" y="120" width="40" height="70" rx="4" fill="#b91c1c" stroke="#f87171"/>
    <text x="115" y="158" font-size="8" font-weight="bold" fill="#fff" text-anchor="middle">Touch</text>

    <text x="75" y="215" font-size="11" font-weight="bold" fill="#f8fafc" text-anchor="middle">Layer 4: Modules</text>
  </g>
  <text x="780" y="235" font-size="22" fill="#64748b" text-anchor="middle">➔</text>

  <!-- Layer 5: Battery & Back Cover -->
  <g transform="translate(800, 110)">
    <rect x="0" y="0" width="140" height="240" rx="8" fill="#713f12" stroke="#eab308" stroke-width="2"/>
    <rect x="15" y="40" width="110" height="130" rx="6" fill="#ca8a04"/>
    <text x="70" y="110" font-size="12" font-weight="bold" fill="#000" text-anchor="middle">200mAh LiPo</text>
    <text x="70" y="195" font-size="13" font-weight="bold" fill="#fef08a" text-anchor="middle">Layer 5: Battery</text>
    <text x="70" y="215" font-size="10" fill="#fde047" text-anchor="middle">+ Clear Backplate</text>
  </g>

  <!-- Bottom Instruction Box -->
  <g transform="translate(60, 390)">
    <rect x="0" y="0" width="880" height="160" rx="10" fill="#1e293b" stroke="#334155" stroke-width="1.5"/>
    <text x="25" y="32" font-size="15" font-weight="bold" fill="#00f0ff">🛠️ Sandwich Assembly Pro-Tips for Maximum Durability:</text>
    
    <text x="25" y="65" font-size="13" fill="#cbd5e1">1. <tspan font-weight="bold" fill="#fff">Cut Perfboard First:</tspan> Cut green perfboard to ~35mm x 55mm so it has a 5mm margin around the 1.3" display.</text>
    <text x="25" y="92" font-size="13" fill="#cbd5e1">2. <tspan font-weight="bold" fill="#fff">Keyring Hole:</tspan> Drill a 3.5mm hole in the top corner of the perfboard BEFORE soldering components.</text>
    <text x="25" y="119" font-size="13" fill="#cbd5e1">3. <tspan font-weight="bold" fill="#fff">Foam Tape Cushion:</tspan> Stick the 200mAh battery behind the ESP32 using double-sided foam tape to absorb drops.</text>
    <text x="25" y="145" font-size="13" fill="#cbd5e1">4. <tspan font-weight="bold" fill="#fff">Clear Conformal Coating:</tspan> Paint solder joints with clear nail polish to make it 100% immune to backpack short-circuits.</text>
  </g>
</svg>
`;

fs.writeFileSync('C:/Users/Gourav/Documents/GitHub/Keychain/assets/wiring_schematic.svg', wiringSvg);
fs.writeFileSync('C:/Users/Gourav/Documents/GitHub/Keychain/assets/tp4056_mod.svg', tpModSvg);
fs.writeFileSync('C:/Users/Gourav/Documents/GitHub/Keychain/assets/sandwich_layout.svg', sandwichSvg);
console.log("SVGs generated successfully!");
