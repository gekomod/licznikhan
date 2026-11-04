#pragma once
#include <Arduino.h>

// Strona WWW (dashboard). Dane: /json (biezacy odczyt), /history?since=<s> (moc co 30 s).
const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html><html lang="pl"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><meta name="theme-color" content="#5b5ff0"><title>Licznik energii</title>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" media="print" onload="this.media='all'">
<style>
:root{--bg:#f4f5fa;--card:#fff;--fg:#151a27;--mut:#6b7285;--line:#e7e9f0;--track:#eef0f5;--soft:#f7f8fb;
--imp:#f26b2e;--imp2:#ffa463;--exp:#12b886;--exp2:#63e6be;--acc:#5b5ff0;--acc2:#8b5cf6;--warn:#f0a400;--bad:#e5484d;--ok:#12b886;
--sh:0 1px 2px #1018280a,0 10px 28px -10px #1018281f;--r:20px}
@media(prefers-color-scheme:dark){:root{--bg:#0b0e15;--card:#141924;--fg:#e8ebf2;--mut:#8b93a7;--line:#232a38;--track:#1e2532;--soft:#181e2a;
--imp:#ff8040;--imp2:#ffb27a;--exp:#20c997;--exp2:#7ef0c8;--acc:#7c80ff;--acc2:#a78bfa;--sh:0 12px 30px -12px #000a}}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--fg);font:15px/1.45 Inter,system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;-webkit-font-smoothing:antialiased}
body:before{content:"";position:fixed;left:0;right:0;top:0;height:380px;pointer-events:none;z-index:-1;
 background:radial-gradient(800px 300px at 10% -15%,color-mix(in srgb,var(--acc) 16%,transparent),transparent),radial-gradient(700px 280px at 95% -25%,color-mix(in srgb,var(--imp) 12%,transparent),transparent)}
.top{position:sticky;top:0;z-index:5;background:color-mix(in srgb,var(--bg) 75%,transparent);backdrop-filter:saturate(1.6) blur(14px);-webkit-backdrop-filter:saturate(1.6) blur(14px);border-bottom:1px solid color-mix(in srgb,var(--line) 70%,transparent)}
.top .in{max-width:1180px;margin:0 auto;padding:12px 20px;display:flex;align-items:center;gap:14px}
#pb{position:absolute;left:0;bottom:-1px;height:2px;width:0;background:linear-gradient(90deg,var(--acc),var(--acc2));opacity:.8}
.logo{flex:none;width:40px;height:40px;border-radius:12px;display:grid;place-items:center;background:linear-gradient(135deg,var(--acc),var(--acc2));box-shadow:0 8px 18px -6px color-mix(in srgb,var(--acc) 70%,transparent)}
.logo svg{width:22px;height:22px;fill:#fff}
h1{font-size:1.15rem;margin:0;letter-spacing:-.015em;font-weight:700}
.sub{color:var(--mut);font-size:.82rem}
.live{margin-left:auto;display:flex;align-items:center;gap:10px}
.dot{width:9px;height:9px;border-radius:50%;background:var(--mut);flex:none}
.dot.ok{background:var(--ok);animation:pulse 2s infinite}.dot.warn{background:var(--warn)}.dot.bad{background:var(--bad)}
@keyframes pulse{0%{box-shadow:0 0 0 0 color-mix(in srgb,var(--ok) 55%,transparent)}70%{box-shadow:0 0 0 9px transparent}100%{box-shadow:0 0 0 0 transparent}}
.wrap{max-width:1180px;margin:0 auto;padding:18px 20px 48px}
h2{font-size:.95rem;font-weight:600;letter-spacing:-.005em;margin:0}
.lbl{font-size:.8rem;color:var(--mut);font-weight:500}
select,button,input{font:inherit;color:var(--fg);background:var(--card);border:1px solid var(--line);border-radius:10px;padding:6px 10px}
select{font-size:.85rem}button{cursor:pointer}
.btn{display:inline-flex;align-items:center;gap:6px;background:var(--soft);font-size:.85rem;font-weight:500;padding:7px 12px;transition:background .2s,border-color .2s}
.btn:hover{border-color:var(--acc)}.btn svg{width:15px;height:15px;stroke:currentColor;fill:none;stroke-width:2;stroke-linecap:round;stroke-linejoin:round}
.chips{display:flex;flex-wrap:wrap;gap:8px;margin-bottom:16px}
.chip{display:inline-flex;align-items:center;gap:7px;background:var(--card);border:1px solid var(--line);border-radius:99px;padding:4px 11px;font-size:.8rem;color:var(--mut)}
.chip i{width:7px;height:7px;border-radius:50%;background:var(--mut)}
.chip.ok i{background:var(--ok)}.chip.warn i{background:var(--warn)}.chip.bad i{background:var(--bad)}
.card{background:var(--card);border:1px solid var(--line);border-radius:var(--r);padding:20px;box-shadow:var(--sh);min-width:0}
.hd{display:flex;flex-wrap:wrap;justify-content:space-between;align-items:center;gap:10px;margin-bottom:12px}
.hd .sub{margin-top:2px}
.note{border-left:4px solid var(--warn);margin-bottom:16px}
.hero{display:grid;gap:16px;grid-template-columns:minmax(290px,1fr) 2fr;margin-bottom:16px}
.tiles{display:grid;gap:16px;grid-template-columns:repeat(2,minmax(0,1fr))}
.tile{display:flex;flex-direction:column}
.th{display:flex;justify-content:space-between;align-items:center;margin-bottom:8px}
.ic{width:34px;height:34px;border-radius:10px;display:grid;place-items:center}
.ic svg{width:18px;height:18px;fill:none;stroke:currentColor;stroke-width:2;stroke-linecap:round;stroke-linejoin:round}
.i-imp{color:var(--imp);background:color-mix(in srgb,var(--imp) 13%,transparent)}.i-exp{color:var(--exp);background:color-mix(in srgb,var(--exp) 13%,transparent)}
.i-acc{color:var(--acc);background:color-mix(in srgb,var(--acc) 13%,transparent)}
.val{font-size:1.95rem;font-weight:700;letter-spacing:-.025em;line-height:1.1;font-variant-numeric:tabular-nums}
.val small{font-size:.5em;color:var(--mut);font-weight:500;letter-spacing:0;margin-left:3px}
.foot{margin-top:auto;padding-top:8px;font-size:.8rem;color:var(--mut);display:flex;flex-wrap:wrap;gap:6px;align-items:center}
.pill{display:inline-block;border-radius:99px;padding:1px 8px;font-size:.75rem;font-weight:600}
.pill.up{color:var(--bad);background:color-mix(in srgb,var(--bad) 12%,transparent)}.pill.dn{color:var(--ok);background:color-mix(in srgb,var(--ok) 13%,transparent)}
.gauge svg{width:100%;max-width:290px;display:block;margin:4px auto 0}
.gv{font-size:36px;font-weight:700;fill:var(--fg);letter-spacing:-1px}.gu{font-size:12px;fill:var(--mut)}
.kv{display:grid;grid-template-columns:1fr auto;gap:6px 12px;font-size:.88rem;margin-top:10px}
.kv span{color:var(--mut)}.kv b{font-weight:600;text-align:right;font-variant-numeric:tabular-nums}
.gauge .kv{border-top:1px solid var(--line);padding-top:12px}
.stack{display:flex;gap:2px;height:8px;border-radius:99px;overflow:hidden;background:var(--track);margin:12px 0 8px}
.stack div{height:100%;transition:width .6s}
.leg{display:flex;flex-wrap:wrap;gap:4px 14px;font-size:.8rem;color:var(--mut)}
.leg i{display:inline-block;width:9px;height:9px;border-radius:3px;margin-right:5px;vertical-align:-1px}
.leg b{color:var(--fg);font-weight:600}
.bar{position:relative;height:10px;border-radius:99px;background:var(--track);overflow:hidden}
.bar .fill{position:absolute;top:0;bottom:0;left:0;border-radius:99px;transition:width .6s,background .3s}
.bar .band{position:absolute;top:0;bottom:0;background:color-mix(in srgb,var(--ok) 15%,transparent)}
.vrow{display:grid;grid-template-columns:30px 1fr 72px;gap:12px;align-items:center;margin:14px 0}
.vrow b{text-align:right;font-variant-numeric:tabular-nums}.vrow span{color:var(--mut);font-weight:600;font-size:.85rem}
.scale{display:flex;justify-content:space-between;font-size:.72rem;color:var(--mut);margin-left:42px;margin-right:84px}
.vh{font-size:.85rem;color:var(--mut);font-weight:600;margin:22px 0 4px;padding-top:14px;border-top:1px solid var(--line)}
#vtab{margin-top:6px}.vexp{display:flex;flex-wrap:wrap;align-items:flex-end;gap:8px;margin-top:12px}
.vexp label{font-size:.75rem;color:var(--mut);display:flex;flex-direction:column;gap:2px}.vexp input{color:var(--fg)}
.vexp form{margin-left:auto}.vexp .vdel{color:var(--bad);border-color:var(--line)}
.seg{display:inline-flex;background:var(--track);border-radius:11px;padding:3px;gap:2px}
.seg button{border:0;border-radius:8px;padding:5px 12px;font-size:.8rem;font-weight:500;background:transparent;color:var(--mut);transition:background .2s,color .2s}
.seg button.on{background:var(--card);color:var(--fg);box-shadow:0 1px 3px #0000001f}
.seg button:disabled{opacity:.35;cursor:default}
.mt{margin-top:16px}
#chart{position:relative;height:250px;touch-action:pan-y}
#chart svg,.bc svg{display:block;width:100%;height:100%}
#chart text,.bc text{fill:var(--mut);font-size:11px}
.bc{position:relative;height:230px;touch-action:pan-y}#mchart{cursor:pointer}
.empty{display:flex;height:100%;align-items:center;justify-content:center;color:var(--mut);text-align:center;padding:20px;font-size:.9rem;background:var(--soft);border-radius:14px}
#tip,.tip{position:absolute;pointer-events:none;background:var(--card);border:1px solid var(--line);border-radius:12px;padding:8px 11px;font-size:.8rem;line-height:1.55;box-shadow:0 12px 30px -8px #0004;white-space:nowrap;display:none;z-index:2}
.kpis{display:grid;grid-template-columns:repeat(auto-fit,minmax(170px,1fr));gap:10px;margin:4px 0 16px}
.kpi{background:var(--soft);border:1px solid var(--line);border-radius:14px;padding:12px 14px}
.kpi span{display:block;color:var(--mut);font-size:.78rem;font-weight:500}
.kpi b{display:block;font-size:1.3rem;font-weight:700;letter-spacing:-.02em;margin-top:2px;font-variant-numeric:tabular-nums}
.kpi small{display:block;color:var(--mut);font-size:.75rem}
.g2{display:grid;gap:16px;grid-template-columns:repeat(auto-fit,minmax(320px,1fr));margin:16px 0}
.w{overflow-x:auto}
table{width:100%;border-collapse:collapse;font-size:.85rem}
th{font-size:.74rem;font-weight:500;color:var(--mut);text-align:left;padding:8px;border-bottom:1px solid var(--line)}
td{padding:9px 8px;border-bottom:1px solid var(--line);text-align:left;vertical-align:top}
tr:last-child td{border-bottom:0}
.n{text-align:right!important;font-variant-numeric:tabular-nums;white-space:nowrap}
#mtab tr[data-k]{cursor:pointer;transition:background .15s}#mtab tr[data-k]:hover td{background:var(--soft)}
#mtab tr.sel td{background:color-mix(in srgb,var(--acc) 9%,transparent)}#mtab tr.sel td:first-child{box-shadow:inset 3px 0 var(--acc)}
.foot2{display:flex;flex-wrap:wrap;justify-content:flex-end;gap:8px;margin-top:12px}
.pins{display:grid;gap:8px;margin:12px 0}
.prow{display:grid;grid-template-columns:62px minmax(30px,90px) 62px 1fr;align-items:center;gap:10px;color:var(--mut)}
.prow b{padding:5px 0;border-radius:8px;border:1.5px solid currentColor;text-align:center;font-size:.8rem;background:color-mix(in srgb,currentColor 10%,transparent)}
.prow i{height:3px;border-radius:2px;background:currentColor}.prow.bad i{background:repeating-linear-gradient(90deg,currentColor 0 6px,transparent 6px 10px)}
.prow small{color:var(--fg);font-size:.82rem}.prow.ok{color:var(--ok)}.prow.warn{color:var(--warn)}.prow.bad{color:var(--bad)}
.prow.hd b{border:0;background:none;font-weight:500;padding:0}
.stres{padding:10px 14px;border-radius:10px;font-size:.88rem;border-left:4px solid var(--mut);background:var(--soft)}
.stres.ok{border-color:var(--ok)}.stres.warn{border-color:var(--warn)}.stres.bad{border-color:var(--bad)}
@media(max-width:560px){.prow{grid-template-columns:62px 1fr 62px}.prow small{grid-column:1/-1;margin:-4px 0 4px}}
.hex{word-break:break-all;font-family:ui-monospace,Consolas,monospace;font-size:.8em}
pre{white-space:pre-wrap;background:var(--soft);border:1px solid var(--line);padding:12px;border-radius:12px;font-size:.78em}
details summary{cursor:pointer;font-weight:600;list-style:none;display:flex;align-items:center;gap:8px}
details summary::-webkit-details-marker{display:none}
details summary:before{content:"";width:7px;height:7px;border-right:2px solid var(--mut);border-bottom:2px solid var(--mut);transform:rotate(-45deg);transition:transform .2s}
details[open] summary:before{transform:rotate(45deg)}
details h3{font-size:.85rem;color:var(--mut);font-weight:600;margin:20px 0 8px}
form label{display:block;margin:10px 0;font-size:.88rem;color:var(--mut)}form input{width:100%;font-family:ui-monospace,Consolas,monospace;margin-top:4px;color:var(--fg)}
form button{background:var(--acc);color:#fff;border-color:var(--acc);font-weight:600;padding:8px 18px}
.sg{display:grid;gap:16px;grid-template-columns:repeat(auto-fit,minmax(min(100%,320px),1fr));margin-top:16px}
.sbox{background:var(--soft);border:1px solid var(--line);border-radius:14px;padding:16px;min-width:0}
details .sbox h3{margin:0 0 4px;color:var(--fg);font-size:.95rem}.sbox .sub{margin-bottom:4px}
.row4{display:grid;grid-template-columns:repeat(auto-fit,minmax(110px,1fr));gap:0 10px}
.mrow{display:flex;justify-content:space-between;align-items:center;gap:10px;padding:7px 0;border-top:1px solid var(--line);font-size:.85rem}
.mrow:first-child{margin-top:12px}.mrow button{padding:3px 10px;font-size:.8rem;background:transparent;color:var(--bad);border-color:var(--line);font-weight:500}
code{font-family:ui-monospace,Consolas,monospace;font-size:.85em;background:var(--track);padding:1px 5px;border-radius:5px;word-break:break-all}
a.btn{color:var(--fg);text-decoration:none;border:1px solid var(--line);border-radius:10px;margin-top:10px}
@media(max-width:820px){.hero{grid-template-columns:1fr}}
@media(max-width:600px){.top .in,.wrap{padding-left:16px;padding-right:16px}.card{padding:16px}.val{font-size:1.7rem}.tiles{gap:12px}
 .tiles .val{font-size:1.35rem}.kpis{grid-template-columns:repeat(2,minmax(0,1fr))}.kpi b{font-size:1.1rem}.hero{gap:12px}.top .sub{display:none}}
@media(max-width:380px){.tiles{grid-template-columns:1fr}}
</style></head><body>
<div class="top"><div class="in"><div class="logo"><svg viewBox="0 0 24 24"><path d="M13.5 2 4.5 13.5h6.5L10 22l9.5-12H13z"/></svg></div>
<div><h1>Licznik energii</h1><div class="sub" id="sub">HAN wireless M-Bus</div></div>
<div class="live"><span class="dot" id="ld" title="Stan odczytu"></span><select id="iv" title="Częstotliwość odświeżania"><option value="2">co 2 s</option><option value="5">co 5 s</option>
<option value="10">co 10 s</option><option value="30">co 30 s</option><option value="60">co 60 s</option></select></div></div><div id="pb"></div></div>
<div class="wrap">
<div class="chips" id="chips"></div>
<div class="card note" id="note" hidden></div>
<div class="hero">
 <div class="card gauge"><div class="hd" style="margin:0"><h2 id="gt">Moc chwilowa</h2><span class="lbl">na żywo</span></div>
  <svg viewBox="0 0 220 142"><defs><linearGradient id="gg" x1="0" x2="1"><stop offset="0" stop-color="var(--imp2)"/><stop offset="1" stop-color="var(--imp)"/></linearGradient></defs>
  <path d="M20 120A90 90 0 0 1 200 120" fill="none" stroke="var(--track)" stroke-width="14" stroke-linecap="round"/>
  <path id="ga" d="M20 120A90 90 0 0 1 200 120" fill="none" stroke="url(#gg)" stroke-width="14" stroke-linecap="round" pathLength="100" stroke-dasharray="100" stroke-dashoffset="100" style="transition:stroke-dashoffset .8s,stroke .3s"/>
  <text x="110" y="100" text-anchor="middle" class="gv" id="gv">–</text><text x="110" y="121" text-anchor="middle" class="gu" id="gu">W</text>
  <text x="20" y="141" text-anchor="middle" class="gu">0</text><text x="200" y="141" text-anchor="middle" class="gu" id="gmax"></text></svg>
  <div class="kv"><span>Pobór</span><b id="pi">–</b><span>Oddawanie</span><b id="pe">–</b><span>Moc maks.</span><b id="pm">–</b><span>Próg mocy</span><b id="pl">–</b></div></div>
 <div class="tiles">
  <div class="card tile"><div class="th"><span class="lbl">Dziś</span><span class="ic i-imp"><svg viewBox="0 0 24 24"><path d="M13 2 4 14h7l-1 8 9-12h-7z"/></svg></span></div>
   <div class="val" id="td">–</div><div class="foot" id="tdf"></div></div>
  <div class="card tile"><div class="th"><span class="lbl" id="tml">Ten miesiąc</span><span class="ic i-acc"><svg viewBox="0 0 24 24"><rect x="3" y="5" width="18" height="16" rx="3"/><path d="M3 10h18M8 3v4M16 3v4"/></svg></span></div>
   <div class="val" id="tm">–</div><div class="foot" id="tmf"></div></div>
  <div class="card tile"><div class="th"><span class="lbl">Energia pobrana · stan</span><span class="ic i-imp"><svg viewBox="0 0 24 24"><path d="M12 3v12m0 0-5-5m5 5 5-5M4 21h16"/></svg></span></div>
   <div class="val" id="ei">–</div><div class="stack" id="si"></div><div class="leg" id="li"></div></div>
  <div class="card tile"><div class="th"><span class="lbl">Energia oddana · stan</span><span class="ic i-exp"><svg viewBox="0 0 24 24"><path d="M12 15V3m0 0L7 8m5-5 5 5M4 21h16"/></svg></span></div>
   <div class="val" id="ee">–</div><div class="stack" id="se"></div><div class="leg" id="le"></div></div>
 </div></div>
<div class="card"><div class="hd"><div><h2>Moc w czasie</h2><div class="sub">średnia z 30 s</div></div>
 <div class="seg" id="rg"><button data-r="900">15 min</button><button data-r="3600">1 h</button><button data-r="10800">3 h</button><button data-r="21600">6 h</button></div></div>
 <div id="chart"></div>
 <div class="leg" style="margin-top:8px"><span><i style="background:var(--imp)"></i>Pobór</span><span><i style="background:var(--exp)"></i>Oddawanie</span></div></div>
<div class="card mt"><div class="hd"><div><h2>Zestawienie miesięczne</h2><div class="sub">z dobowych stanów licznika zapisanych w ESP</div></div>
 <div class="seg" id="mr"><button data-m="6">6 mies.</button><button data-m="12">12 mies.</button><button data-m="24">24 mies.</button></div></div>
 <div class="kpis" id="kpi"></div>
 <div id="mchart" class="bc"></div>
 <div class="leg" style="margin-top:8px"><span><i style="background:var(--imp)"></i>Pobór</span><span><i style="background:var(--exp)"></i>Oddanie</span><span>jaśniejszy słupek – okres w toku, niepełny lub szacowany</span></div>
 <div class="w mt"><table id="mtab"></table></div>
 <div class="foot2"><button class="btn" id="csvm"><svg viewBox="0 0 24 24"><path d="M12 3v12m0 0-4-4m4 4 4-4M5 21h14"/></svg>CSV – miesiące</button>
 <button class="btn" id="csvd"><svg viewBox="0 0 24 24"><path d="M12 3v12m0 0-4-4m4 4 4-4M5 21h14"/></svg>CSV – dni</button></div></div>
<div class="card mt"><div class="hd"><div><h2 id="dt">Zużycie dzienne</h2><div class="sub">kliknij miesiąc na wykresie lub w tabeli</div></div>
 <div class="seg"><button id="dprev" title="Poprzedni miesiąc">‹ poprz.</button><button id="dnext" title="Następny miesiąc">nast. ›</button></div></div>
 <div id="dchart" class="bc"></div></div>
<div class="g2">
 <div class="card" id="napiecia"><div class="hd"><h2>Napięcia faz</h2><span class="lbl">norma 207–253 V</span></div><div id="volt"></div><div class="scale"><span>200 V</span><span>230 V</span><span>260 V</span></div>
  <h3 class="vh">Odchylenia od normy</h3><div class="sub" id="vsum">–</div><div class="w"><table id="vtab"></table></div>
  <div class="vexp"><label>Od<input type="date" id="vfrom"></label><label>Do<input type="date" id="vto"></label>
   <button class="btn" id="vxml"><svg viewBox="0 0 24 24"><path d="M12 3v12m0 0-4-4m4 4 4-4M5 21h14"/></svg>XML</button>
   <button class="btn" id="vpdf"><svg viewBox="0 0 24 24"><path d="M12 3v12m0 0-4-4m4 4 4-4M5 21h14"/></svg>PDF</button>
   <form method="post" action="/voltclear" onsubmit="return confirm('Usunąć wszystkie zapisane odchylenia napięcia?')"><button class="btn vdel">Wyczyść</button></form></div></div>
 <div class="card"><div class="hd"><h2>Licznik</h2></div><div class="kv" id="info"></div></div>
</div>
<details class="card mt" id="ustawienia"><summary>Ustawienia</summary>
 <div class="sub" style="margin-top:6px">Zapis wymaga hasła administratora (użytkownik <b>admin</b>).</div>
 <div class="sg">
  <form method="post" action="/config" class="sbox"><h3>Ceny energii</h3>
   <div class="sub">Cena brutto za 1 kWh razem z dystrybucją (składniki zależne od zużycia). Puste T2–T4 = jak T1.</div>
   <div class="row4"><label>T1 / całodobowa<input name="p1" id="p1" inputmode="decimal" placeholder="zł/kWh"></label><label>T2<input name="p2" id="p2" inputmode="decimal" placeholder="zł/kWh"></label>
   <label>T3<input name="p3" id="p3" inputmode="decimal" placeholder="zł/kWh"></label><label>T4<input name="p4" id="p4" inputmode="decimal" placeholder="zł/kWh"></label></div>
   <label>Opłaty stałe (abonament, opłaty sieciowe), zł/mies.<input name="fee" id="fee" inputmode="decimal"></label><button>Zapisz ceny</button></form>
  <div class="sbox"><form method="post" action="/reading"><h3>Stan licznika z rachunku</h3>
   <div class="sub">Uzupełnia historię sprzed montażu czujnika. Zużycie między stanami jest rozkładane równo na dni.</div>
   <div class="row4"><label>Data odczytu<input type="date" name="date" required></label><label>Pobór, kWh<input name="imp" inputmode="decimal" required></label>
   <label>T1, kWh<input name="t1" inputmode="decimal" placeholder="opcjonalnie"></label><label>T2, kWh<input name="t2" inputmode="decimal" placeholder="opcjonalnie"></label></div>
   <label>Oddanie, kWh<input name="exp" inputmode="decimal" placeholder="opcjonalnie"></label><button>Dodaj stan</button></form><div id="manual"></div></div>
  <form method="post" action="/config" class="sbox"><h3>Licznik</h3>
   <label>Klucz AES (32 znaki hex, z TAURON eLicznik)<input name="key" placeholder="pozostaw puste, aby nie zmieniać" autocomplete="off"></label>
   <label>ID licznika (puste = dowolny)<input name="id" id="mid"></label><button>Zapisz</button></form>
  <div class="sbox"><h3>Mini-cloud</h3><div class="sub" id="cst">–</div>
   <div id="capp" style="display:none;margin:8px 0">Aplikacja Android – login: <b id="cu"></b> · hasło: <b id="cp">••••••</b> <button type="button" class="btn" onclick="showApp()">Pokaż hasło</button></div>
   <form method="post" action="/cloudregister" id="creg" onsubmit="return confirm('Zarejestrować licznik w mini-cloud? Otrzymasz login i hasło do aplikacji.')"><button>Rejestruj w mini-cloud</button></form>
   <form method="post" action="/config" id="cdel" style="display:none" onsubmit="return confirm('Usunąć rejestrację? ESP przestanie wysyłać odczyty.')"><input type="hidden" name="cloud_forget" value="1"><button class="btn vdel">Usuń rejestrację</button></form></div>
  <form method="post" action="/config" class="sbox"><h3>Hasło administratora</h3>
   <label>Nowe hasło (4–32 znaki)<input type="password" name="pass" autocomplete="new-password" required></label>
   <label>Powtórz hasło<input type="password" name="pass2" autocomplete="new-password" required></label><button>Zmień hasło</button>
   <div class="sub" style="margin-top:8px">To samo hasło obowiązuje przy aktualizacji OTA – zmień też <code>--auth</code> w platformio.ini.</div></form>
  <div class="sbox"><h3>Aktualizacja oprogramowania</h3>
   <div class="sub">Z przeglądarki: wybierz plik <code>.pio/build/nodemcuv2/firmware.bin</code>.<br>Z PlatformIO przez Wi-Fi: <code>python -m platformio run -e ota -t upload</code></div>
   <a class="btn" href="/update">Wgraj firmware…</a></div>
 </div></details>
<details class="card mt" id="diag"><summary>Diagnostyka</summary>
 <h3>Test połączeń CC1101</h3>
 <div class="sub">Sprawdza przewody między NodeMCU a modułem radiowym: zwarcia, brak odpowiedzi modułu, komunikację SPI. Odbiór jest przerywany na ułamek sekundy.</div>
 <div id="st" class="pins"></div><button class="btn" id="stb">Sprawdź połączenia</button>
 <h3>Radio</h3><div class="kv" id="rf"></div><div class="sub" id="rfn" style="margin-top:8px"></div>
 <h3>Odebrane urządzenia</h3><div class="w"><table id="dev"></table></div>
 <h3>Rekordy</h3><pre id="rec">–</pre>
 <h3>Ostatnio wysłane do mini-cloud</h3><pre id="cpl">–</pre></details>
</div><script>
const $=i=>document.getElementById(i),TC=['#5b5ff0','#f26b2e','#a855f7','#f0a400'];
const esc=s=>String(s??'').replace(/[&<>"]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]));
const fx=(v,d=0)=>v==null?'–':v.toLocaleString('pl-PL',{minimumFractionDigits:d,maximumFractionDigits:d});
const pw=v=>v==null?'–':Math.abs(v)>=10000?fx(v/1000,1)+' kW':fx(v)+' W';
const ls=(k,d)=>{try{return+localStorage.getItem(k)||d}catch(e){return d}},ss=(k,v)=>{try{localStorage.setItem(k,v)}catch(e){}};
function nice(v){const p=Math.pow(10,Math.floor(Math.log10(v))),n=v/p;return(n<=1?1:n<=2?2:n<=2.5?2.5:n<=5?5:10)*p}
const dur=s=>s<60?s+' s':s<3600?Math.floor(s/60)+' min':s<86400?Math.floor(s/3600)+' h '+Math.floor(s%3600/60)+' min':Math.floor(s/86400)+' d '+Math.floor(s%86400/3600)+' h';
let ivs=ls('iv',5),range=ls('rg',3600),H=[],hLast=0,hSync=0,up=0,upAt=0,J=null,tmr,next=0;
$('iv').value=ivs;$('iv').onchange=()=>{ivs=+$('iv').value;ss('iv',ivs);sched(0)};
function setRange(r){range=r;ss('rg',r);[...$('rg').children].forEach(b=>b.classList.toggle('on',+b.dataset.r==r));draw()}
$('rg').onclick=e=>{if(e.target.dataset.r)setRange(+e.target.dataset.r)};
function sched(ms){clearTimeout(tmr);next=Date.now()+ms;tmr=setTimeout(tick,ms)}
async function tick(){await load();sched(ivs*1000)}
(function pb(){$('pb').style.width=Math.max(0,Math.min(100,100-(next-Date.now())/(ivs*10)))+'%';requestAnimationFrame(pb)})();
let skew=0,pf=0;const wall=t=>t*1000+skew;
async function load(){
 try{const j=await(await fetch('/json',{cache:'no-store'})).json();
  if(j.uptime_s<up){H=[];hLast=0;hSync=0}up=j.uptime_s;upAt=Date.now();J=j;if(j.time)skew=Date.now()-j.time*1000;
  if(Date.now()-hSync>25000){try{const h=await(await fetch('/history?since='+hLast,{cache:'no-store'})).json();
   h.s.forEach(s=>H.push({t:s[0],i:s[1],e:s[2]}));if(H.length)hLast=H[H.length-1].t;
   H=H.filter(p=>p.t>(j.time||1e10)-22000);hSync=Date.now()}catch(e){}}
  const last=D.length?D[D.length-1][0]:0;
  if(Date.now()-dSync>(last==todayN()?600000:60000)){try{D=(await(await fetch('/days',{cache:'no-store'})).json()).d;dSync=Date.now();mDrawn=0}catch(e){}}
  if(Date.now()-vSync>60000)await loadVolt();
  render(j)}catch(e){chips(null)}}
function chip(c,t){return`<span class="chip ${c}"><i></i>${t}</span>`}
function chips(j){
 if(!j){$('chips').innerHTML=chip('bad','Brak połączenia z ESP');$('ld').className='dot bad';return}
 const m=j.meter,a=m?m.age_s:null;
 $('chips').innerHTML=chip(j.radio_ok?'ok':'bad',j.radio_ok?'Radio CC1101 OK':'Brak CC1101')+
  chip(j.key_set?'ok':'bad',j.key_set?'Klucz AES ustawiony':'Brak klucza AES')+
  chip(a==null?'warn':a<120?'ok':a<600?'warn':'bad',a==null?'Brak odczytu':'Odczyt '+dur(a)+' temu')+
  (m?chip(m.rssi>-80?'ok':m.rssi>-95?'warn':'bad','Sygnał '+m.rssi+' dBm'):'')+
  chip(j.crc_errors>j.frames/2?'warn':'','Ramki '+j.frames+' · CRC '+j.crc_errors);
 $('ld').className='dot '+(a==null?'warn':a<120?'ok':a<600?'warn':'bad')}
function stack(id,lid,m,k){
 const t=[1,2,3,4].map(n=>m['import_t'.replace('import',k)+n+'_kwh']).map((v,i)=>({v,i})).filter(x=>x.v>0),s=t.reduce((a,x)=>a+x.v,0);
 $(id).innerHTML=t.map(x=>`<div style="width:${x.v/s*100}%;background:${TC[x.i]}"></div>`).join('');
 $(id).style.display=t.length?'':'none';
 $(lid).innerHTML=t.length?t.map(x=>`<span><i style="background:${TC[x.i]}"></i>T${x.i+1} <b>${fx(x.v,1)}</b> kWh (${fx(x.v/s*100)}%)</span>`).join(''):'Brak podziału na taryfy';}
function bigKwh(v){if(v==null)return'–';const[a,b]=fx(v,3).split(',');return a+'<small>,'+b+' kWh</small>'}
function render(j){
 chips(j);const m=j.meter||{};
 $('sub').textContent=j.meter?'Licznik '+(m.serial||m.id)+(m.manufacturer?' · '+m.manufacturer:'')+' · HAN wM-Bus':'HAN wireless M-Bus';
 let n='';if(!j.radio_ok)n='Nie wykryto modułu CC1101 – sprawdź połączenia SPI i zasilanie 3,3 V. <a href="#diag" onclick="openDiag()">Uruchom test połączeń</a>';
 else if(!j.meter)n=j.key_set?'Czekam na pierwszy poprawny odczyt z licznika…':'Wpisz klucz AES w sekcji „Ustawienia” na dole strony.';
 if(m.message)n+=(n?' ':'')+'Komunikat licznika: '+esc(m.message);
 $('note').hidden=!n;$('note').innerHTML=n;
 const imp=m.power_import_w,ex=m.power_export_w,gen=ex>0&&!(imp>0),v=gen?ex:imp;
 const sc=nice(Math.max(5000,m.power_max_w||0,v||0)),f=v==null?0:Math.min(1,v/sc);
 $('gt').textContent=gen?'Oddawanie do sieci':'Moc chwilowa';
 $('ga').style.strokeDashoffset=100-f*100;$('ga').style.stroke=gen?'var(--exp)':f<.6?'url(#gg)':f<.85?'var(--warn)':'var(--bad)';
 $('gv').textContent=v==null?'–':v>=10000?fx(v/1000,1):fx(v);$('gu').textContent=(v>=10000?'kW':'W')+'';
 $('gmax').textContent=fx(sc/1000,sc%1000?1:0)+' kW';$('pi').textContent=pw(imp);$('pe').textContent=pw(ex);$('pm').textContent=pw(m.power_max_w);$('pl').textContent=pw(m.power_limit_w);
 $('ei').innerHTML=bigKwh(m.import_kwh);$('ee').innerHTML=bigKwh(m.export_kwh);stack('si','li',m,'import');stack('se','le',m,'export');
 const vs=[1,2,3].map(p=>[p,m['voltage_l'+p]]).filter(x=>x[1]!=null);
 $('volt').innerHTML=vs.length?vs.map(([p,u])=>{const ok=u>=207&&u<=253,w=Math.max(2,Math.min(100,(u-200)/60*100));
  return`<div class="vrow"><span>L${p}</span><div class="bar"><div class="band" style="left:11.7%;width:76.6%"></div><div class="fill" style="width:${w}%;background:${ok?'var(--ok)':'var(--bad)'}"></div></div><b>${fx(u,1)} V</b></div>`}).join('')
  :'<div class="sub">Brak danych o napięciach</div>';
 const inf=[['Numer seryjny',m.serial],['ID radiowe',m.id],['Producent',m.manufacturer],['Czas licznika',m.datetime],
  ['Ostatni odczyt',m.age_s!=null?dur(m.age_s)+' temu':null],['Sygnał',m.rssi!=null?m.rssi+' dBm':null],['Czas pracy ESP',dur(j.uptime_s)],['Filtr ID',j.meter_filter||'brak'],
  ['Zapisane doby',j.days_stored==null?null:j.days_stored<0?'błąd pamięci':String(j.days_stored)]];
 $('info').innerHTML=inf.filter(x=>x[1]).map(x=>`<span>${x[0]}</span><b>${esc(x[1])}</b>`).join('');
 rfStats(j);$('mid').placeholder=j.meter_filter||'np. 12345678';$('rec').textContent=m.records||'–';$('cpl').textContent=j.cloud&&j.cloud.last_payload?JSON.stringify(j.cloud.last_payload,null,1):'–';
 $('dev').innerHTML='<tr><th>ID</th><th>Prod.</th><th>Ramek</th><th>RSSI</th><th>Ost.</th><th>Status</th><th>Ostatnia ramka</th></tr>'+
  j.devices.sort((a,b)=>b.count-a.count).map(d=>`<tr><td>${esc(d.id)}</td><td>${esc(d.mfr)}</td><td>${d.count}</td><td>${d.rssi}</td><td>${dur(d.age_s)}</td><td>${esc(d.result)}</td><td class="hex">${esc(d.hex)}</td></tr>`).join('');
 if(!pf){pf=1;const pv=v=>v==null?'':String(v).replace('.',',');[1,2,3,4].forEach(t=>$('p'+t).value=pv(j.price&&j.price[t-1]));$('fee').value=j.fee_month>0?pv(j.fee_month):''}
 if(j.cloud){const c=j.cloud;$('cst').textContent=c.registered?'Zarejestrowano · Device ID '+c.id+' · wysłano '+c.sent+' (co '+c.interval_s+' s)'+(c.last_age_s!=null?' · ostatnia próba '+dur(c.last_age_s)+' temu, kod '+c.last_code+(c.error?' – '+c.error:''):''):'Nie zarejestrowano – odczyty nie są wysyłane.';$('capp').style.display=c.registered&&c.app_user?'':'none';$('cu').textContent=c.app_user||'';$('creg').style.display=c.registered?'none':'';$('cdel').style.display=c.registered?'':'none'}
 draw();if(Date.now()-mDrawn>30000)drawMonths()}
async function showApp(){try{const r=await fetch('/cloudapp');if(r.ok){const a=await r.json();$('cp').textContent=a.pass||'–'}}catch(e){}}
function rfStats(j){
 $('rf').innerHTML=[['Moduł CC1101',j.radio_ok?'wykryty':'brak'],['Ramki poprawne',j.frames-j.crc_errors],['Ramki z błędem CRC',j.crc_errors],
  ['Synchronizacje (w tym szum)',j.sync],['Poziom szumu',j.noise_dbm!=null?j.noise_dbm+' dBm':'–']].map(x=>`<span>${x[0]}</span><b>${esc(x[1])}</b>`).join('');
 let t='';if(!j.radio_ok)t='';else if(j.sync_c1>=3&&j.sync_c1*1000>j.sync_bad)t='Wykryto transmisje w trybie C1 ('+j.sync_c1+') – ten firmware odbiera tylko tryb T1.';
 else if(!j.frames&&j.uptime_s>180)t='Przez '+dur(j.uptime_s)+' nie odebrano żadnej ramki wM-Bus. Jeśli test połączeń jest OK – licznik nie nadaje (interfejs HAN niewłączony przez TAURON) albo problem z anteną / modułem (np. wersja 433 MHz).';
 else if(j.frames>0&&j.crc_errors>j.frames/2)t='Większość ramek z błędem CRC – słaby sygnał: sprawdź antenę, zbliż moduł do licznika.';
 $('rfn').textContent=t}
let stRun=0;
async function selftest(){const b=$('stb');stRun=1;b.disabled=true;b.textContent='Sprawdzam…';
 try{const r=await(await fetch('/selftest',{method:'POST'})).json();
  const sum={ok:'Wszystkie połączenia w porządku.',warn:'Są drobne uwagi – zobacz żółte pozycje.',bad:'Wykryto problem z połączeniem – przewody oznaczone na czerwono.'}[r.result];
  $('st').innerHTML=`<div class="stres ${r.result}">${sum}${r.noise_dbm!=null?' Poziom szumu radia: '+r.noise_dbm+' dBm.':''}</div>`+
   '<div class="prow hd"><b>CC1101</b><span></span><b>NodeMCU</b><span></span></div>'+
   r.pins.map(p=>`<div class="prow ${p.s}"><b>${p.cc}</b><i></i><b>${p.esp}</b><small>${esc(p.m||'')}</small></div>`).join('')}
 catch(e){$('st').innerHTML='<div class="stres bad">Nie udało się wykonać testu.</div>'}
 b.disabled=false;b.textContent='Sprawdź ponownie';load()}
$('stb').onclick=selftest;
$('diag').addEventListener('toggle',()=>{if($('diag').open&&!stRun)selftest()});
function openDiag(){$('diag').open=true;setTimeout(()=>$('diag').scrollIntoView(),100)}
let P=[],X,Yi,t0;
function draw(){
 const el=$('chart'),W=el.clientWidth||600,Hh=el.clientHeight||240,L=46,R=10,T=10,B=24,now=Date.now();t0=now-range*1000;
 P=H.map(p=>({x:wall(p.t),i:p.i,e:p.e}));const m=J&&J.meter;
 if(m&&m.age_s<120&&J.time){const x=wall(J.time-m.age_s);if(!P.length||x>P[P.length-1].x+1000)P.push({x,i:m.power_import_w,e:m.power_export_w})}
 P=P.filter(p=>p.x>=t0-60000);
 if(!P.length){el.innerHTML='<div class="empty">Brak danych z tego okresu.<br>Wykres wypełnia się automatycznie (próbka co 30 s).</div>';return}
 const mx=nice(Math.max(100,...P.map(p=>Math.max(p.i||0,p.e||0)))*1.1);
 X=x=>L+(x-t0)/(range*1000)*(W-L-R);Yi=v=>T+(1-v/mx)*(Hh-T-B);const base=Yi(0);
 const segs=k=>{const out=[];let c=[];P.forEach((p,n)=>{const gap=n&&p.x-P[n-1].x>150000;if(p[k]==null||gap){if(c.length)out.push(c);c=[]}if(p[k]!=null)c.push([X(p.x),Yi(p[k])])});if(c.length)out.push(c);return out};
 const ln=s=>s.map(c=>'M'+c.map(q=>q[0].toFixed(1)+','+q[1].toFixed(1)).join('L')).join('');
 const ar=s=>s.map(c=>c.length>1?'M'+c[0][0].toFixed(1)+','+base+'L'+c.map(q=>q[0].toFixed(1)+','+q[1].toFixed(1)).join('L')+'L'+c[c.length-1][0].toFixed(1)+','+base+'Z':'').join('');
 const si=segs('i'),se=segs('e');let g='';
 for(let k=0;k<=4;k++){const v=mx*k/4,y=Yi(v);g+=`<line x1="${L}" x2="${W-R}" y1="${y}" y2="${y}" stroke="var(--line)"/><text x="${L-6}" y="${y+4}" text-anchor="end">${mx>=2000?fx(v/1000,v%1000?(v%100?2:1):0)+' kW':fx(v)+' W'}</text>`}
 const st=range<=900?300:range<=3600?900:range<=10800?1800:3600;
 for(let t=Math.ceil(t0/1000/st)*st*1000;t<=now;t+=st*1000){const x=X(t),d=new Date(t);
  g+=`<line x1="${x}" x2="${x}" y1="${T}" y2="${base}" stroke="var(--line)" stroke-dasharray="2 4"/><text x="${x}" y="${Hh-6}" text-anchor="middle">${String(d.getHours()).padStart(2,'0')}:${String(d.getMinutes()).padStart(2,'0')}</text>`}
 el.innerHTML=`<svg viewBox="0 0 ${W} ${Hh}"><defs><linearGradient id="gi" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="var(--imp)" stop-opacity=".3"/><stop offset="1" stop-color="var(--imp)" stop-opacity="0"/></linearGradient></defs>${g}
<path d="${ar(si)}" fill="url(#gi)"/><path d="${ln(si)}" fill="none" stroke="var(--imp)" stroke-width="2.25" stroke-linejoin="round" stroke-linecap="round"/>
<path d="${ln(se)}" fill="none" stroke="var(--exp)" stroke-width="2.25" stroke-linejoin="round" stroke-linecap="round"/>
<g id="hv" style="display:none"><line id="hl" y1="${T}" y2="${base}" stroke="var(--mut)" stroke-dasharray="3 3"/><circle id="hi" r="5" fill="var(--imp)" stroke="var(--card)" stroke-width="2"/><circle id="he" r="5" fill="var(--exp)" stroke="var(--card)" stroke-width="2"/></g></svg><div id="tip"></div>`}
function hover(ev){
 if(!P.length||!X)return;const el=$('chart'),r=el.getBoundingClientRect(),cx=ev.clientX-r.left;
 let b=P[0];P.forEach(p=>{if(Math.abs(X(p.x)-cx)<Math.abs(X(b.x)-cx))b=p});
 const x=X(b.x),hv=$('hv'),tip=$('tip');if(!hv)return;hv.style.display='';
 $('hl').setAttribute('x1',x);$('hl').setAttribute('x2',x);
 [['hi','i'],['he','e']].forEach(([id,k])=>{const c=$(id);c.style.display=b[k]==null?'none':'';if(b[k]!=null){c.setAttribute('cx',x);c.setAttribute('cy',Yi(b[k]))}});
 const d=new Date(b.x);tip.innerHTML=`<b>${d.toLocaleTimeString('pl-PL')}</b><br><span style="color:var(--imp)">●</span> Pobór ${pw(b.i)}<br><span style="color:var(--exp)">●</span> Oddawanie ${pw(b.e)}`;
 tip.style.display='block';tip.style.left=Math.min(r.width-tip.offsetWidth-4,Math.max(4,x+12))+'px';tip.style.top='8px'}
$('chart').addEventListener('pointermove',hover);$('chart').addEventListener('pointerdown',hover);
$('chart').addEventListener('pointerleave',()=>{const hv=$('hv');if(hv)hv.style.display='none';if($('tip'))$('tip').style.display='none'});
// ---- Zestawienie miesięczne: dobowe stany licznika z /days + bieżący odczyt ----
const MN=['sty','lut','mar','kwi','maj','cze','lip','sie','wrz','paź','lis','gru'],
 MF=['Styczeń','Luty','Marzec','Kwiecień','Maj','Czerwiec','Lipiec','Sierpień','Wrzesień','Październik','Listopad','Grudzień'];
const dnum=(y,m,d)=>Math.round(Date.UTC(y,m,d)/864e5),dof=n=>new Date(n*864e5),dimOf=k=>new Date(Date.UTC(Math.floor(k/100),k%100,0)).getUTCDate();
const todayN=()=>{const n=new Date();return n.getFullYear()*1e4+(n.getMonth()+1)*100+n.getDate()};
const mlab=(k,l)=>l?MF[k%100-1]+' '+Math.floor(k/100):MN[k%100-1]+(k%100==1?" '"+String(k).slice(2,4):'');
const kwh=(v,d=1)=>v==null?'–':fx(v/1000,d);
let D=[],dSync=0,mRange=ls('mr',12),selM=0,mDrawn=0,DAYS={},MON=[];
// Zużycie doby = różnica kolejnych stanów; przy przerwie w odczytach rozkładane równo na dni (szacunek)
function calc(){
 const p=D.map(r=>({k:dnum(Math.floor(r[0]/1e4),Math.floor(r[0]/100)%100-1,r[0]%100),v:r.slice(1)})),m=J&&J.meter;
 if(m&&m.import_kwh!=null&&m.age_s<900){const n=new Date(),k=dnum(n.getFullYear(),n.getMonth(),n.getDate())+(n.getHours()*3600+n.getMinutes()*60+n.getSeconds())/86400;
  if(!p.length||k>p[p.length-1].k)p.push({k,live:1,v:['import_kwh','import_t1_kwh','import_t2_kwh','import_t3_kwh','import_t4_kwh','export_kwh'].map(x=>m[x]==null?null:Math.round(m[x]*1000))})}
 const days={},M={},pr=J&&J.price&&J.price[0]>0?J.price:null;
 for(let n=1;n<p.length;n++){const a=p[n-1],b=p[n],len=b.k-a.k,dv=c=>a.v[c]!=null&&b.v[c]!=null&&b.v[c]>=a.v[c]?b.v[c]-a.v[c]:null;
  let ic=null;if(pr){const tt=[1,2,3,4].filter(c=>dv(c)!=null);ic=tt.length?tt.reduce((s,c)=>s+dv(c)/1000*(pr[c-1]||pr[0]),0):dv(0)!=null?dv(0)/1000*pr[0]:null}
  for(let d=Math.floor(a.k);d<b.k;d++){const ov=Math.min(d+1,b.k)-Math.max(d,a.k);if(ov<=0)continue;
   const o=days[d]||(days[d]={d,v:[null,null,null,null,null,null],cov:0,cost:null,est:false,live:false});
   o.cov+=ov;if(ic!=null)o.cost=(o.cost||0)+ic*ov/len;if(len>1.001)o.est=true;if(b.live)o.live=true;
   a.v.forEach((x,c)=>{const y=b.v[c];if(x!=null&&y!=null&&y>=x)o.v[c]=(o.v[c]||0)+(y-x)*ov/len})}}
 Object.values(days).forEach(o=>{const t=dof(o.d),key=t.getUTCFullYear()*100+t.getUTCMonth()+1,
  q=M[key]||(M[key]={key,v:[null,null,null,null,null,null],cov:0,cost:null,n:0,est:false,live:false});
  q.n++;if(o.cost!=null)q.cost=(q.cost||0)+o.cost;q.cov+=o.cov;q.est=q.est||o.est;q.live=q.live||o.live;o.v.forEach((x,c)=>{if(x!=null)q.v[c]=(q.v[c]||0)+x})});
 DAYS=days;MON=Object.values(M).sort((a,b)=>a.key-b.key)}
const zl=v=>v==null?'–':fx(v,2)+' zł',mc=q=>q.cost==null?null:q.cost+(J&&J.fee_month||0);
function tipv(v,cost){const t=[1,2,3,4].filter(c=>v[c]>0);
 return`<span style="color:var(--imp)">●</span> Pobór ${kwh(v[0],2)} kWh`+(t.length>1?'<br>'+t.map(c=>'T'+c+' '+kwh(v[c],2)).join(' · '):'')+
  (v[5]>0?`<br><span style="color:var(--exp)">●</span> Oddanie ${kwh(v[5],2)} kWh`:'')+(cost!=null?'<br>Koszt ≈ '+zl(cost):'')}
// Wykres słupkowy: it=[{key,lab,a,b,dim,tip}], a/b w Wh
function bars(id,it,sel,pick){
 const el=$(id),W=el.clientWidth||600,Hh=el.clientHeight||220,L=46,R=8,T=18,B=24;
 if(!it.some(x=>x.a!=null||x.b!=null)){el.onpointermove=el.onpointerdown=el.onclick=null;
  el.innerHTML='<div class="empty">Brak danych.<br>Zestawienie tworzy się z dobowych stanów licznika zapisywanych w pamięci ESP.</div>';return}
 const n=it.length,two=it.some(x=>x.b>0),mx=nice(Math.max(100,...it.map(x=>Math.max(x.a||0,x.b||0)))*1.1),
  cw=(W-L-R)/n,bw=Math.max(2,Math.min(26,cw*(two?.36:.62))),Y=v=>T+(1-v/mx)*(Hh-T-B),base=Y(0),ev=Math.ceil(30/cw);
 let g='';const i0=it.findIndex(x=>x.key==sel);
 g+=`<defs>${[['i','imp'],['e','exp']].map(([k,c])=>`<linearGradient id="${id}-${k}" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="var(--${c})"/><stop offset="1" stop-color="var(--${c}2)"/></linearGradient>`).join('')}</defs>`;
 if(i0>=0)g+=`<rect x="${L+i0*cw+1}" y="${T-6}" width="${cw-2}" height="${base-T+6}" rx="8" fill="var(--acc)" opacity=".1"/>`;
 g+=`<text x="${L-6}" y="10" text-anchor="end">kWh</text>`;
 for(let k=0;k<=4;k++){const v=mx*k/4/1000,y=Y(v*1000);g+=`<line x1="${L}" x2="${W-R}" y1="${y}" y2="${y}" stroke="var(--line)"/><text x="${L-6}" y="${y+4}" text-anchor="end">${fx(v,Number.isInteger(v)?0:v<1?2:1)}</text>`}
 it.forEach((x,i)=>{const cx=L+i*cw+cw/2,op=x.dim?' opacity=".45"':'',
  r=(v,x0,c)=>{if(!(v>0))return'';const y=Y(v),q=Math.min(5,bw/2,base-y);
   return`<path d="M${x0.toFixed(1)},${base}V${(y+q).toFixed(1)}Q${x0.toFixed(1)},${y.toFixed(1)} ${(x0+q).toFixed(1)},${y.toFixed(1)}H${(x0+bw-q).toFixed(1)}Q${(x0+bw).toFixed(1)},${y.toFixed(1)} ${(x0+bw).toFixed(1)},${(y+q).toFixed(1)}V${base}Z" fill="url(#${id}-${c})"${op}/>`};
  g+=two?r(x.a,cx-bw-1,'i')+r(x.b,cx+1,'e'):r(x.a,cx-bw/2,'i');
  if(i%ev==0)g+=`<text x="${cx}" y="${Hh-6}" text-anchor="middle">${x.lab}</text>`});
 el.innerHTML=`<svg viewBox="0 0 ${W} ${Hh}">${g}<rect id="${id}-h" y="${T}" width="${cw}" height="${base-T}" rx="8" fill="var(--fg)" opacity=".05" style="display:none"/></svg><div class="tip"></div>`;
 const tip=el.querySelector('.tip'),hl=$(id+'-h'),at=e=>{const i=Math.floor((e.clientX-el.getBoundingClientRect().left-L)/cw);return i>=0&&i<n?i:-1};
 el.onpointerleave=()=>{hl.style.display='none';tip.style.display='none'};
 el.onpointermove=el.onpointerdown=e=>{const i=at(e);if(i<0)return el.onpointerleave();const x=L+i*cw;hl.setAttribute('x',x);hl.style.display='';
  tip.innerHTML=it[i].tip;tip.style.display='block';tip.style.left=Math.min(el.clientWidth-tip.offsetWidth-4,Math.max(4,x+cw+6))+'px';tip.style.top='8px'};
 el.onclick=e=>{const i=at(e);if(i>=0&&pick)pick(it[i].key)}}
const unit=(v,d)=>v==null?'–':fx(v/1000,d)+'<small>kWh</small>',
 pill=(v,ref)=>ref>0&&v!=null?`<span class="pill ${v>ref?'up':'dn'}">${v>ref?'+':''}${fx((v/ref-1)*100)}%</span>`:'';
function kpis(){
 const n=new Date(),ti=dnum(n.getFullYear(),n.getMonth(),n.getDate()),td=DAYS[ti],yd=DAYS[ti-1],c=MON[MON.length-1];
 $('td').innerHTML=unit(td&&td.v[0],2);$('tdf').innerHTML=[td&&td.cost!=null?'≈ '+zl(td.cost):'',yd&&yd.v[0]!=null?'wczoraj '+kwh(yd.v[0],2)+' kWh':''].filter(x=>x).join(' · ');
 if(!c){$('kpi').innerHTML=$('tm').innerHTML=$('tmf').innerHTML='';$('tm').textContent='–';return}
 const ck=n.getFullYear()*100+n.getMonth()+1,cur=c.key==ck,pv=MON.find(q=>q.key==(c.key%100==1?c.key-89:c.key-1)),ly=MON.find(q=>q.key==c.key-100),dm=dimOf(c.key),
  avg=c.v[0]!=null&&c.cov>0?c.v[0]/c.cov:null,fc=cur&&avg!=null&&c.cov>=1?avg*dm:null,
  k=(t,v,s)=>`<div class="kpi"><span>${t}</span><b>${v}</b>${s?'<small>'+s+'</small>':''}</div>`;
 $('tml').textContent=cur?'Ten miesiąc':mlab(c.key,1);$('tm').innerHTML=unit(c.v[0],1);
 const fee=J&&J.fee_month||0,fcc=fc!=null&&c.cost!=null?c.cost/c.cov*dm+fee:null;
 $('tmf').innerHTML=(fc!=null?'prognoza ≈ '+kwh(fc,0)+' kWh'+(fcc!=null?' · '+zl(fcc):'')+' '+pill(fc,pv&&pv.v[0]):c.n<dm?'dane z '+c.n+' z '+dm+' dni':'')+(fc==null&&c.cost!=null?' · ≈ '+zl(mc(c)):'');
 let h=k('Średnio na dobę',kwh(avg,2)+' kWh',mlab(c.key,1));
 if(fc!=null)h+=k('Prognoza na miesiąc','≈ '+kwh(fc,0)+' kWh',pv&&pv.v[0]>0?(fc>=pv.v[0]?'+':'')+fx((fc/pv.v[0]-1)*100)+'% wzgl. poprzedniego':'');
 if(pv)h+=k('Poprzedni miesiąc',kwh(pv.v[0])+' kWh',mlab(pv.key,1)+(pv.est?' · szacunek':''));
 if(ly)h+=k('Rok temu',kwh(ly.v[0])+' kWh',mlab(ly.key,1)+(ly.est?' · szacunek':''));
 if(c.cost!=null)h+=k('Koszt w miesiącu','≈ '+zl(mc(c)),(fcc!=null?'prognoza ≈ '+zl(fcc):'')+(fee?(fcc!=null?' · ':'')+'z opłatami stałymi '+zl(fee):''));
 if(c.v[5]>0)h+=k('Oddane w miesiącu',kwh(c.v[5])+' kWh','bilans '+kwh(c.v[0]-c.v[5])+' kWh');
 $('kpi').innerHTML=h}
function monthTable(){
 if(!MON.length){$('mtab').innerHTML='';return}
 const tr=[1,2,3,4].filter(c=>MON.some(q=>q.v[c]>0)),tt=tr.length>1?tr:[],ex=MON.some(q=>q.v[5]>0),co=MON.some(q=>q.cost!=null);
 $('mtab').innerHTML=`<tr><th>Miesiąc</th><th class="n">Pobór kWh</th>${tt.map(c=>`<th class="n">T${c}</th>`).join('')}${ex?'<th class="n">Oddanie</th><th class="n">Bilans</th>':''}${co?'<th class="n">Koszt</th>':''}<th class="n">Śr./dobę</th><th class="n">Dni</th></tr>`+
  MON.slice().reverse().map(q=>`<tr data-k="${q.key}"${q.key==selM?' class="sel"':''}><td>${mlab(q.key,1)}${q.live?' <span class="sub">(w toku)</span>':''}</td><td class="n"><b>${q.est?'≈ ':''}${kwh(q.v[0])}</b></td>${tt.map(c=>`<td class="n">${kwh(q.v[c])}</td>`).join('')}${ex?`<td class="n">${kwh(q.v[5])}</td><td class="n">${q.v[0]!=null&&q.v[5]!=null?kwh(q.v[0]-q.v[5]):'–'}</td>`:''}${co?`<td class="n">${zl(mc(q))}</td>`:''}<td class="n">${q.v[0]!=null&&q.cov?kwh(q.v[0]/q.cov,2):'–'}</td><td class="n">${q.n}/${dimOf(q.key)}</td></tr>`).join('')}
function dayChart(){
 const i=MON.findIndex(q=>q.key==selM);$('dprev').disabled=i<=0;$('dnext').disabled=i<0||i>=MON.length-1;
 if(!selM){$('dt').textContent='Zużycie dzienne';bars('dchart',[],0);return}
 const y=Math.floor(selM/100),m=selM%100,it=[];$('dt').textContent='Zużycie dzienne – '+mlab(selM,1);
 for(let d=1;d<=dimOf(selM);d++){const o=DAYS[dnum(y,m-1,d)],ds=new Date(y,m-1,d).toLocaleDateString('pl-PL',{weekday:'short',day:'numeric',month:'long'});
  it.push({key:d,lab:d,a:o?o.v[0]:null,b:o?o.v[5]:null,dim:o&&(o.est||o.live),
   tip:`<b>${ds}</b>`+(o?(o.est?' (szacunek)':o.live?' (w toku)':'')+'<br>'+tipv(o.v,o.cost):'<br>brak danych')})}
 bars('dchart',it,0)}
function manualList(){
 const ds=n=>String(n).replace(/(\d{4})(\d\d)(\d\d)/,'$1-$2-$3');
 $('manual').innerHTML=D.filter(r=>r[7]==1).reverse().map(r=>`<form method="post" action="/reading" class="mrow" onsubmit="return confirm('Usunąć stan z ${ds(r[0])}?')"><input type="hidden" name="date" value="${ds(r[0])}"><input type="hidden" name="del" value="1">
  <span>${ds(r[0])} · <b>${kwh(r[1],3)} kWh</b>${r[6]!=null?' · oddanie '+kwh(r[6],3)+' kWh':''}</span><button>Usuń</button></form>`).join('')}
function drawMonths(){
 mDrawn=Date.now();calc();manualList();
 if(!MON.some(q=>q.key==selM))selM=MON.length?MON[MON.length-1].key:0;
 kpis();monthTable();dayChart();
 bars('mchart',MON.slice(-mRange).map(q=>{const inc=q.n<dimOf(q.key);return{key:q.key,lab:mlab(q.key),a:q.v[0],b:q.v[5],dim:q.est||q.live||inc,
  tip:`<b>${mlab(q.key,1)}</b>${q.live?' (w toku)':q.est?' (szacunek)':inc?' (niepełny)':''}<br>`+tipv(q.v,mc(q))}}),selM,k=>{selM=k;drawMonths()})}
function setMR(r){mRange=r;ss('mr',r);[...$('mr').children].forEach(b=>b.classList.toggle('on',+b.dataset.m==r));drawMonths()}
$('mr').onclick=e=>{if(e.target.dataset.m)setMR(+e.target.dataset.m)};
$('mtab').onclick=e=>{const r=e.target.closest('tr[data-k]');if(r){selM=+r.dataset.k;drawMonths()}};
$('dprev').onclick=()=>{const i=MON.findIndex(q=>q.key==selM);if(i>0){selM=MON[i-1].key;drawMonths()}};
$('dnext').onclick=()=>{const i=MON.findIndex(q=>q.key==selM);if(i>=0&&i<MON.length-1){selM=MON[i+1].key;drawMonths()}};
function dl(name,text,type){const a=document.createElement('a');a.href=URL.createObjectURL(new Blob([text],{type}));
 a.download=name;a.click();setTimeout(()=>URL.revokeObjectURL(a.href),1000)}
function csv(name,rows){dl(name,'﻿'+rows.map(r=>r.join(';')).join('\r\n'),'text/csv')}
// Odchylenia napiecia: VL = [{s,e,p,h,mn,mx,o}] (czas unix, faza, przepiecie, min/maks V, trwa)
let VL=[],vSync=0,VN=[207,253];
async function loadVolt(){try{const j=await(await fetch('/voltlog',{cache:'no-store'})).json();VN=[j.min,j.max];
 VL=j.e.map(r=>({s:r[0],e:r[1],p:r[2],h:r[3],mn:r[4]/10,mx:r[5]/10,o:r[6]==1}));vSync=Date.now();voltLog()}catch(e){}}
const dts=t=>new Date(t*1000).toLocaleString('pl-PL',{dateStyle:'short',timeStyle:'medium'}),vk=o=>o.h?'przepięcie':'zaniżenie',
 vu=v=>fx(v,1)+' V',isoD=d=>new Date(d-d.getTimezoneOffset()*60000).toISOString().slice(0,10);
function voltLog(){
 const now=Date.now()/1000,n=s=>VL.filter(o=>o.e>now-s).length;
 $('vsum').textContent=VL.length?`Zapisanych zdarzeń: ${VL.length} · ostatnia doba: ${n(86400)} · 7 dni: ${n(604800)}`:'Brak zapisanych odchyleń.';
 $('vtab').innerHTML=VL.length?'<tr><th>Początek</th><th>Faza</th><th>Rodzaj</th><th class="n">Czas</th><th class="n">Min–maks</th></tr>'+
  VL.slice().sort((a,b)=>b.s-a.s).slice(0,8).map(o=>`<tr><td>${dts(o.s)}</td><td>L${o.p}</td><td style="color:var(--${o.h?'warn':'bad'})">${vk(o)}${o.o?' <span class="sub">(trwa)</span>':''}</td><td class="n">${dur(o.e-o.s)}</td><td class="n">${fx(o.mn,1)}–${vu(o.mx)}</td></tr>`).join(''):''}
function vRange(){const f=$('vfrom').value,t=$('vto').value,a=f?new Date(f+'T00:00').getTime()/1000:0,b=t?new Date(t+'T00:00').getTime()/1000+86400:4e9;
 return{f,t,ev:VL.filter(o=>o.e>=a&&o.s<b).sort((x,y)=>x.s-y.s)}}
function vStats(ev){return[1,2,3].map(p=>{const e=ev.filter(o=>o.p==p);return{p,n:e.length,lo:e.filter(o=>!o.h).length,hi:e.filter(o=>o.h).length,
 t:e.reduce((a,o)=>a+o.e-o.s,0),mn:e.length?Math.min(...e.map(o=>o.mn)):null,mx:e.length?Math.max(...e.map(o=>o.mx)):null}})}
const meterName=()=>{const m=J&&J.meter||{};return m.serial||m.id||''};
$('vxml').onclick=async()=>{await loadVolt();const r=vRange(),iso=t=>new Date(t*1000).toISOString(),v=x=>x.toFixed(1);
 dl(`napiecia-odchylenia_${r.f||'start'}_${r.t||'dzis'}.xml`,`<?xml version="1.0" encoding="UTF-8"?>
<raportNapiec licznik="${esc(meterName())}" od="${r.f}" do="${r.t}" wygenerowano="${new Date().toISOString()}" normaMinV="${VN[0]}" normaMaxV="${VN[1]}">
 <podsumowanie>
${vStats(r.ev).map(s=>`  <faza nr="${s.p}" zdarzen="${s.n}" zanizen="${s.lo}" przepiec="${s.hi}" czasTrwania_s="${s.t}"${s.n?` minV="${v(s.mn)}" maxV="${v(s.mx)}"`:''}/>`).join('\n')}
 </podsumowanie>
 <zdarzenia liczba="${r.ev.length}">
${r.ev.map(o=>`  <zdarzenie faza="${o.p}" rodzaj="${o.h?'przepiecie':'zanizenie'}" poczatek="${iso(o.s)}" koniec="${iso(o.e)}" czasTrwania_s="${o.e-o.s}" minV="${v(o.mn)}" maxV="${v(o.mx)}" trwa="${o.o}"/>`).join('\n')}
 </zdarzenia>
</raportNapiec>
`,'application/xml')};
$('vpdf').onclick=async()=>{const w=open('','_blank');if(!w)return alert('Zezwól na wyskakujące okna, aby utworzyć PDF.');
 await loadVolt();const r=vRange(),row=a=>'<tr>'+a.map(c=>`<td>${c}</td>`).join('')+'</tr>';
 w.document.write(`<!doctype html><html lang="pl"><head><meta charset="utf-8"><title>Odchylenia napięcia ${r.f} – ${r.t}</title><style>
body{font:12px Arial,sans-serif;margin:24px;color:#000}h1{font-size:18px;margin:0 0 4px}h2{font-size:14px;margin:18px 0 6px}
table{border-collapse:collapse;width:100%}th,td{border:1px solid #999;padding:4px 6px;text-align:left}th{background:#eee}tr{page-break-inside:avoid}.s{color:#555}@page{margin:15mm}</style></head><body>
<h1>Raport odchyleń napięcia</h1><div class="s">Licznik ${esc(meterName()||'–')} · okres ${r.f||'od początku'} – ${r.t||'dziś'} · norma ${VN[0]}–${VN[1]} V (230 V ±10%, PN-EN 50160) · wygenerowano ${dts(Date.now()/1000)}</div>
<h2>Podsumowanie</h2><table><tr><th>Faza</th><th>Zdarzeń</th><th>Zaniżeń</th><th>Przepięć</th><th>Łączny czas</th><th>Min. napięcie</th><th>Maks. napięcie</th></tr>
${vStats(r.ev).map(s=>row(['L'+s.p,s.n,s.lo,s.hi,s.n?dur(s.t):'–',s.n?vu(s.mn):'–',s.n?vu(s.mx):'–'])).join('')}</table>
<h2>Zdarzenia (${r.ev.length})</h2>${r.ev.length?`<table><tr><th>Lp.</th><th>Faza</th><th>Rodzaj</th><th>Początek</th><th>Koniec</th><th>Czas trwania</th><th>Min.</th><th>Maks.</th></tr>
${r.ev.map((o,i)=>row([i+1,'L'+o.p,vk(o),dts(o.s),o.o?'trwa':dts(o.e),dur(o.e-o.s),vu(o.mn),vu(o.mx)])).join('')}</table>`:'<p>Brak odchyleń w wybranym okresie.</p>'}
<p class="s">Początek i koniec zdarzenia wyznaczone z odczytów licznika (wysyłanych co kilka–kilkanaście sekund).</p></body></html>`);
 w.document.close();w.focus();setTimeout(()=>w.print(),400)};
{const d=new Date();$('vto').value=isoD(d);d.setDate(d.getDate()-30);$('vfrom').value=isoD(d)}
const cv=v=>v==null?'':(v/1000).toFixed(3).replace('.',','),cz=v=>v==null?'':v.toFixed(2).replace('.',','),note=o=>o.live?'w toku':o.est?'szacunek':'';
$('csvm').onclick=()=>{calc();csv('licznik-miesiace.csv',[['miesiac','pobor_kWh','T1_kWh','T2_kWh','T3_kWh','T4_kWh','oddanie_kWh','koszt_zl','dni_z_danymi','dni_w_miesiacu','uwagi'],
 ...MON.map(q=>[Math.floor(q.key/100)+'-'+String(q.key%100).padStart(2,'0'),...q.v.map(cv),cz(mc(q)),q.n,dimOf(q.key),note(q)])])};
$('csvd').onclick=()=>{calc();csv('licznik-dni.csv',[['data','pobor_kWh','T1_kWh','T2_kWh','T3_kWh','T4_kWh','oddanie_kWh','koszt_energii_zl','uwagi'],
 ...Object.values(DAYS).sort((a,b)=>a.d-b.d).map(o=>[dof(o.d).toISOString().slice(0,10),...o.v.map(cv),cz(o.cost),note(o)])])};
addEventListener('resize',()=>{draw();drawMonths()});setRange(range);setMR(mRange);sched(0);
if(location.hash=='#ustawienia'){$('ustawienia').open=true;setTimeout(()=>$('ustawienia').scrollIntoView(),300)}
if(location.hash=='#napiecia')setTimeout(()=>$('napiecia').scrollIntoView(),300);
</script></body></html>)HTML";
