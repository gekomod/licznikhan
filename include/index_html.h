#pragma once
#include <Arduino.h>

// Strona WWW (dashboard). Dane: /json (biezacy odczyt), /history?since=<s> (moc co 30 s), /days, /voltlog.
const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html><html lang="pl"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><meta name="theme-color" id="mtc" content="#f2f3f6"><title>Licznik energii</title>
<script>try{var t=localStorage.getItem('th');document.documentElement.dataset.th=(t?t=='d':matchMedia('(prefers-color-scheme:dark)').matches)?'dark':'light'}catch(e){}</script>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Manrope:wght@400;500;600;700;800&display=swap" media="print" onload="this.media='all'">
<style>
:root{--bg:#f2f3f6;--card:#fff;--text:#14161b;--muted:#6b7080;--bdr:rgba(20,22,27,.07);--track:#ebedf1;--inset:#f4f5f8;
--shadow:0 1px 2px rgba(20,25,40,.04),0 10px 30px rgba(20,25,40,.05);--okt:oklch(.52 .13 155);--wrt:oklch(.58 .14 65);
--o:oklch(.74 .17 55);--g:oklch(.72 .17 155);--v:oklch(.64 .18 295);--t:oklch(.72 .13 200);--y:oklch(.83 .16 85);--b:oklch(.64 .14 250);
--red:oklch(.66 .21 25);--amb:oklch(.75 .16 70);--p2:oklch(.70 .16 330);--p3:oklch(.70 .14 185);--hero:#0f1522;color-scheme:light}
[data-th=dark]{--bg:#0a0d14;--card:#121722;--text:#eef1f6;--muted:#8c93a4;--bdr:rgba(255,255,255,.07);--track:#1b2130;--inset:#0d111a;--shadow:none;
--okt:var(--g);--wrt:var(--amb);color-scheme:dark}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--text);font:15px/1.5 Manrope,system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;font-variant-numeric:tabular-nums;-webkit-font-smoothing:antialiased;transition:background .3s,color .3s}
a{color:var(--b)}button{font:inherit}
@keyframes flow{to{stroke-dashoffset:-36.2}}
.topbar{position:sticky;top:0;z-index:10;height:3px;overflow:hidden}
.rb{display:block;height:100%;width:0;background:linear-gradient(90deg,var(--y),var(--o))}
.wrap{max-width:1200px;margin:0 auto;padding:0 24px}
header{display:flex;align-items:center;gap:16px;flex-wrap:wrap;padding:14px 0 10px}
.brand{display:flex;align-items:center;gap:12px;margin-right:auto}
.logo{width:34px;height:34px;border-radius:10px;background:linear-gradient(135deg,var(--y),var(--o));display:flex;align-items:center;justify-content:center;box-shadow:0 6px 18px color-mix(in oklch,var(--o) 35%,transparent)}
.logo svg{width:20px;height:20px;fill:none;stroke:#1a1206;stroke-width:2;stroke-linecap:round;stroke-linejoin:round}
.brand div{display:flex;flex-direction:column;line-height:1.15}.brand strong{font-weight:800;font-size:18px;letter-spacing:-.02em}
.brand small{font-size:12px;color:var(--muted)}.brand small b{color:var(--text)}
.seg{display:flex;gap:2px;padding:3px;background:var(--track);border-radius:10px}
.seg button{border:0;border-radius:8px;padding:6px 12px;font-size:13px;font-weight:600;cursor:pointer;background:transparent;color:var(--muted);white-space:nowrap}
.seg button.on{background:var(--card);color:var(--text);box-shadow:0 1px 3px rgba(0,0,0,.12)}
nav.seg{padding:4px;border-radius:12px}nav.seg button{border-radius:9px;padding:6px 14px}
.icb{width:34px;height:34px;border-radius:10px;border:0;background:var(--track);color:var(--text);cursor:pointer;display:flex;align-items:center;justify-content:center}
.icb svg{width:18px;height:18px;fill:none;stroke:currentColor;stroke-width:1.75;stroke-linecap:round;stroke-linejoin:round}
.chips{display:flex;flex-wrap:wrap;gap:6px;padding-bottom:14px;font-size:12px;color:var(--muted)}
.chip{display:flex;align-items:center;gap:7px;padding:5px 11px;border-radius:99px;background:var(--card);border:1px solid var(--bdr)}.chip b{color:var(--text)}
.chip .dot{width:7px;height:7px;border-radius:50%}.mini{width:48px;height:5px;border-radius:99px;background:var(--track);overflow:hidden;display:block}
.sig{display:flex;align-items:flex-end;gap:1.5px;height:10px}.sig i{width:2.5px;border-radius:1px;background:var(--muted);opacity:.4}.sig i.on{opacity:1;background:var(--g)}
.today{margin-left:auto;display:flex;align-items:center}
main{display:flex;flex-direction:column;gap:16px;padding:4px 0 56px}
.note{border-radius:14px;padding:12px 16px;background:color-mix(in oklch,var(--amb) 14%,var(--card));border:1px solid color-mix(in oklch,var(--amb) 35%,transparent);font-size:14px}
.card{background:var(--card);border:1px solid var(--bdr);border-radius:14px;box-shadow:var(--shadow);padding:20px;display:flex;flex-direction:column;gap:14px;min-width:0}
.ch{display:flex;justify-content:space-between;align-items:center;gap:12px;flex-wrap:wrap}.ch>div:first-child:not(.leg){display:flex;flex-direction:column;gap:2px}
h2{font-size:17px;font-weight:700;letter-spacing:-.01em;margin:0}h3{margin:0;font-size:18px;font-weight:700}
h1{margin:0;font-size:26px;font-weight:800;letter-spacing:-.02em}
.sub{font-size:13px;color:var(--muted)}
.row{display:flex;flex-wrap:wrap;gap:20px}.row>*{min-width:0}
.leg{display:flex;gap:16px;flex-wrap:wrap;font-size:12px;color:var(--muted)}.leg span{display:flex;align-items:center;gap:6px}
.sw{width:10px;height:10px;border-radius:3px;display:inline-block}
.pill{font-size:11px;font-weight:700;padding:2px 8px;border-radius:99px;white-space:nowrap}
.btn{border:0;border-radius:10px;padding:7px 12px;font-size:13px;font-weight:600;cursor:pointer;background:var(--track);color:var(--text);text-decoration:none;display:inline-flex;align-items:center;gap:6px;align-self:flex-start}
.btn:disabled{opacity:.4;cursor:default}.btn.pri{background:var(--b);color:#fff}.btn.gh{background:transparent}.btn.del{background:transparent;color:var(--red)}
.empty{display:flex;align-items:center;justify-content:center;text-align:center;color:var(--muted);font-size:13px;padding:24px;border-radius:12px;background:var(--inset);min-height:120px}
.tip{position:absolute;pointer-events:none;padding:10px 14px;border-radius:12px;background:var(--text);color:var(--bg);font-size:12px;white-space:nowrap;display:none;z-index:3;line-height:1.5}
/* hero */
.hero{--c:var(--o);--c2:var(--y);border-radius:18px;padding:20px 24px;color:#f2f4f8;display:flex;flex-wrap:wrap;gap:20px;align-items:center;box-shadow:0 20px 50px rgba(10,15,30,.25);
background:radial-gradient(700px 380px at 30% 0%,color-mix(in oklch,var(--y) 16%,transparent),transparent 65%),radial-gradient(600px 360px at 100% 100%,color-mix(in oklch,var(--g) 12%,transparent),transparent 60%),var(--hero)}
.hero.gen{--c:var(--g);--c2:var(--t)}
.fw{flex:1 1 380px;min-width:0;display:flex;flex-direction:column;align-items:center;gap:30px}
.flow{width:100%;position:relative;aspect-ratio:600/280;max-width:460px}
.tot{display:flex;align-items:stretch;justify-content:center;gap:22px;padding:12px 22px;border-radius:14px;background:rgba(255,255,255,.04);border:1px solid rgba(255,255,255,.07)}
.tot div{display:flex;flex-direction:column;align-items:center;gap:1px}.tot i{width:1px;background:rgba(255,255,255,.1)}
.tot span{font-size:10px;font-weight:600;letter-spacing:.06em;text-transform:uppercase;color:rgba(242,244,248,.5)}
.tot b{font-size:17px;font-weight:700;letter-spacing:-.01em}.tot small{font-size:12px;font-weight:500;color:rgba(242,244,248,.5);margin-left:3px}
.flow>svg{position:absolute;inset:0;width:100%;height:100%}
.ft{fill:none;stroke:rgba(255,255,255,.09);stroke-width:6;stroke-linecap:round}
.fg{fill:none;stroke:var(--c);stroke-width:2;stroke-linecap:round;filter:drop-shadow(0 0 6px var(--c));transition:opacity .5s}
.fl{fill:none;stroke:#fff;stroke-width:5;stroke-linecap:round;stroke-dasharray:.1 18;animation:flow 1s linear infinite;transition:opacity .5s}
.hero.gen .fl{animation-direction:reverse}
.fb{position:absolute;top:28%;transform:translate(-50%,-50%);font-size:11px;font-weight:700;padding:2px 8px;border-radius:99px;background:var(--hero);border:1px solid var(--c);color:var(--c);white-space:nowrap;transition:opacity .5s}
.node{position:absolute;top:50%;width:15.5%;transform:translate(-50%,-50%);display:flex;flex-direction:column;align-items:center}
.nc{width:100%;aspect-ratio:1;border-radius:50%;background:color-mix(in oklch,var(--nc) 14%,#0e1420);border:1.5px solid var(--nc);display:flex;align-items:center;justify-content:center;
box-shadow:0 0 0 7px color-mix(in oklch,var(--nc) 10%,transparent),0 0 40px color-mix(in oklch,var(--nc) 35%,transparent)}
.nc svg{width:42%;fill:none;stroke:var(--nc);stroke-width:1.6;stroke-linecap:round;stroke-linejoin:round}
.nl{position:absolute;top:calc(100% + 10px);display:flex;flex-direction:column;align-items:center;white-space:nowrap}
.nl span{font-size:10px;font-weight:600;letter-spacing:.06em;text-transform:uppercase;color:rgba(242,244,248,.55)}
.nl b{font-size:15px;font-weight:700;letter-spacing:-.01em}.nl small{font-size:11px;color:rgba(242,244,248,.5)}
.hr{flex:1.3 1 420px;min-width:0;display:flex;flex-direction:column;gap:14px}
.hk{font-size:13px;font-weight:600;letter-spacing:.08em;text-transform:uppercase;color:rgba(242,244,248,.55)}
.hseg{background:rgba(255,255,255,.07);border-radius:9px}.hseg button{border-radius:7px;padding:4px 9px;font-size:12px;color:rgba(242,244,248,.55)}
.hseg button.on{background:rgba(255,255,255,.16);color:#fff;box-shadow:none}
.dir{align-self:flex-start;font-size:12px;font-weight:700;padding:3px 10px;border-radius:99px;background:color-mix(in oklch,var(--c) 20%,transparent);color:var(--c)}
.pw{display:flex;align-items:baseline;gap:10px}.pw b{font-size:56px;font-weight:800;line-height:1;letter-spacing:-.04em;color:var(--c)}
.pw span{font-size:20px;font-weight:600;color:rgba(242,244,248,.5)}.hsub{font-size:13px;color:rgba(242,244,248,.65)}
.gauge{position:relative;height:6px;border-radius:99px;background:rgba(255,255,255,.08)}
.gauge i{position:absolute;left:0;top:0;bottom:0;border-radius:99px;background:linear-gradient(90deg,var(--c2),var(--c));box-shadow:0 0 16px var(--c);transition:width .6s}
.gauge em{position:absolute;top:-5px;bottom:-5px;width:2px;border-radius:2px;background:rgba(242,244,248,.7)}
.gl{display:flex;justify-content:space-between;gap:10px;font-size:12px;color:rgba(242,244,248,.5)}
.hst{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:8px}
.hs{padding:10px 12px;border-radius:12px;background:rgba(255,255,255,.05);border:1px solid rgba(255,255,255,.07);display:flex;flex-direction:column;gap:6px;min-width:0}
.hs>span{font-size:11px;color:rgba(242,244,248,.6)}.hs>small{font-size:11px;color:rgba(242,244,248,.5)}
.hs div{display:flex;align-items:baseline;gap:5px}.hs b{font-size:20px;font-weight:700;letter-spacing:-.02em}.hs u{text-decoration:none;font-size:13px;color:rgba(242,244,248,.55)}
.hb{height:5px;border-radius:99px;background:rgba(255,255,255,.08)}.hb i{display:block;height:100%;border-radius:99px}
/* wykresy */
#chart{position:relative;height:230px;touch-action:pan-y;cursor:crosshair}#chart svg{display:block;width:100%;height:100%;overflow:visible}
#chart text{fill:var(--muted);font-size:11px;font-family:inherit}
.badge{padding:4px 10px;border-radius:99px;font-size:12px;font-weight:700;background:color-mix(in oklch,var(--k) 16%,transparent);color:var(--k)}
.stats{display:grid;grid-template-columns:repeat(auto-fit,minmax(140px,1fr));gap:8px}
.st{display:flex;flex-direction:column;gap:2px;padding:10px 14px;border-radius:10px;background:var(--inset)}.st span{font-size:11px;color:var(--muted)}.st b{font-size:16px;font-weight:700}
.dc{position:relative;padding-left:34px;padding-top:22px}
.gline{position:absolute;left:34px;right:0;border-top:1px solid var(--bdr)}.glab{position:absolute;left:0;transform:translateY(-50%);font-size:10px;color:var(--muted)}
.avgl{position:absolute;left:34px;right:0;border-top:1.5px dashed var(--o);pointer-events:none;z-index:1}
.dbars{position:relative;display:flex;gap:4px;align-items:flex-end;height:100px}
.db{flex:1 1 0;min-width:0;height:100%;display:flex;align-items:flex-end;justify-content:center;position:relative}
.db i{width:100%;max-width:18px;border-radius:4px 4px 1px 1px;background:color-mix(in oklch,var(--o) 40%,var(--card));transition:background .15s}
.db i.hi{background:var(--o)}.db i.dm{opacity:.5}.db:hover i{background:var(--text);opacity:1}
.db .tt{display:none;position:absolute;left:50%;transform:translateX(-50%);padding:4px 8px;border-radius:8px;background:var(--text);color:var(--bg);font-size:11px;font-weight:700;white-space:nowrap;z-index:4}
.db:hover .tt{display:block}.db:first-child .tt,.db:nth-child(2) .tt{left:0;transform:none}.db:last-child .tt,.db:nth-last-child(2) .tt{left:auto;right:0;transform:none}
.db .rd{position:absolute;left:50%;top:-22px;bottom:0;border-left:2px solid var(--b);z-index:2;pointer-events:none}
.db .rd span{position:absolute;top:0;left:4px;font-size:10px;font-weight:700;color:var(--b);white-space:nowrap;background:var(--card);padding:0 3px;border-radius:4px}
.db .rd.rt span{left:auto;right:4px}
.dlab{display:flex;gap:4px;padding-top:6px}.dlab span{flex:1 1 0;min-width:0;text-align:center;font-size:10px;color:var(--muted)}.dlab .we{color:var(--red);font-weight:700}
.mbars{display:flex;gap:4px;align-items:stretch}
.mbars button{flex:1 1 0;min-width:0;padding:8px 2px;border:0;border-radius:12px;background:transparent;cursor:pointer;display:flex;flex-direction:column;align-items:center;gap:8px;color:var(--muted)}
.mbars button.on{background:color-mix(in oklch,var(--o) 12%,transparent);color:var(--text)}.mbars button.on .ml,.mbars button.on .mv{font-weight:700}
.mb{height:180px;width:100%;display:flex;flex-direction:column;align-items:center;justify-content:flex-end;gap:5px}
.mv{writing-mode:vertical-rl;transform:rotate(180deg);font-size:11px;font-weight:500;white-space:nowrap}
.mb i{width:56%;max-width:44px;border-radius:6px 6px 2px 2px;background:linear-gradient(180deg,var(--o),var(--y))}
.ml{font-size:12px;font-weight:500;white-space:nowrap}
.mdet{border-radius:12px;background:var(--inset);padding:16px;display:flex;flex-direction:column;gap:12px}
.mdet .ch b{font-size:15px}.mi{display:grid;grid-template-columns:repeat(auto-fit,minmax(130px,1fr));gap:10px}
.mi div{display:flex;flex-direction:column;gap:2px}.mi span{font-size:11px;color:var(--muted)}.mi b{font-size:20px;font-weight:700;letter-spacing:-.02em}
.donut{position:relative;width:150px;height:150px;margin:0 auto;flex:none}.donut svg{display:block;transform:rotate(-90deg)}
.donut div{position:absolute;inset:0;display:flex;flex-direction:column;align-items:center;justify-content:center}.donut b{font-size:26px;font-weight:800;letter-spacing:-.03em;line-height:1}
.tl{display:flex;flex-direction:column;gap:12px;flex:1 1 260px}.tl>div{display:flex;flex-direction:column;gap:5px}
.tl .ch{font-size:13px}.tl .ch span{display:flex;align-items:center;gap:8px}
.bar{height:6px;border-radius:99px;background:var(--track)}.bar i{display:block;height:100%;border-radius:99px}
/* napiecia */
.phs{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:12px}
.ph{border-radius:12px;padding:14px 16px;background:color-mix(in oklch,var(--pc) 8%,var(--card));border:1px solid color-mix(in oklch,var(--pc) 25%,transparent);display:flex;flex-direction:column;gap:10px}
.ph .nm{display:flex;align-items:center;gap:8px;font-weight:800;font-size:14px;color:var(--pc)}
.ph .nm i{width:10px;height:10px;border-radius:50%;background:var(--pc);box-shadow:0 0 0 3px color-mix(in oklch,var(--pc) 25%,transparent)}
.ph .vv{display:flex;align-items:baseline;gap:5px}.ph .vv b{font-size:30px;font-weight:800;letter-spacing:-.03em;line-height:1}
.vb{position:relative;height:6px;border-radius:99px;background:var(--track)}
.vb .nb{position:absolute;left:11.67%;right:11.67%;top:0;bottom:0;border-radius:99px;background:color-mix(in oklch,var(--g) 25%,transparent)}
.vb .rg{position:absolute;top:0;bottom:0;border-radius:99px;background:var(--pc);opacity:.45}
.vb .pt{position:absolute;top:50%;width:12px;height:12px;margin:-6px 0 0 -6px;border-radius:50%;background:var(--pc);border:2px solid var(--card);transition:left .6s}
.vmm{display:flex;justify-content:space-between;font-size:11px;color:var(--muted)}.vmm b{color:var(--text)}
.vch{position:relative;padding-left:40px;flex:2 1 520px}.vch svg{width:100%;height:180px;display:block}
.vch .lb{position:absolute;left:0;transform:translateY(-50%);font-size:11px;color:var(--muted)}
.ticks{display:flex;justify-content:space-between;font-size:11px;color:var(--muted);padding-top:6px}
.vempty{position:absolute;left:40px;right:0;top:0;height:180px;display:flex;align-items:center;justify-content:center;text-align:center;font-size:13px;color:var(--muted)}
.dvs{flex:1 1 300px;display:flex;flex-direction:column;min-width:0}
.dv{display:flex;align-items:center;gap:10px;font-size:13px;padding:9px 0;border-top:1px solid var(--bdr)}.dv .r{margin-left:auto}
.vexp{display:flex;flex-wrap:wrap;align-items:flex-end;gap:8px;padding-top:12px;border-top:1px solid var(--bdr)}
.vexp label{font-size:11px;color:var(--muted);display:flex;flex-direction:column;gap:3px}.vexp .in{min-height:32px;padding:4px 8px;font-size:13px}
/* ustawienia / diagnostyka */
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(100%,340px),1fr));gap:20px}
.grid .card{padding:24px}.grid form{display:flex;flex-direction:column;gap:14px;flex:1}
.ttl{display:flex;flex-direction:column;gap:4px;padding:8px 0}
.f label{display:block;font-size:12px;color:var(--muted);margin-bottom:5px}
.in{width:100%;min-height:38px;padding:7px 10px;font:inherit;font-size:14px;color:var(--text);background:var(--inset);border:1px solid var(--bdr);border-radius:10px}
.in:focus{outline:2px solid var(--b);outline-offset:-1px}.mono{font-family:ui-monospace,Consolas,monospace;font-size:13px}
.g4{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:8px}.g2{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:8px}
.mt-a{margin-top:auto}
.kv{display:flex;flex-direction:column}.kv div{display:flex;justify-content:space-between;gap:12px;font-size:13px;padding:8px 0;border-top:1px solid var(--bdr)}
.kv span{color:var(--muted)}.kv b{font-weight:700;text-align:right;word-break:break-all}
.mrow{display:flex;justify-content:space-between;align-items:center;gap:10px;padding:7px 0;border-top:1px solid var(--bdr);font-size:13px}
code{font-family:ui-monospace,Consolas,monospace;font-size:.85em;background:var(--track);padding:1px 5px;border-radius:5px;word-break:break-all}
.dark{grid-column:1/-1;background:var(--hero);color:rgba(242,244,248,.8);border-radius:18px;padding:22px 26px;display:flex;flex-direction:column;gap:10px}
.dark h3{font-size:16px;color:#f2f4f8}.dark pre{margin:0;font-size:12px;line-height:1.8;white-space:pre-wrap;word-break:break-all;font-family:ui-monospace,Consolas,monospace}
.w{overflow-x:auto}table{width:100%;border-collapse:collapse;font-size:13px}
th{font-size:11px;font-weight:600;color:var(--muted);text-align:left;padding:8px;border-bottom:1px solid var(--bdr);text-transform:uppercase;letter-spacing:.04em}
td{padding:8px;border-bottom:1px solid var(--bdr);vertical-align:top}.hex{word-break:break-all;font-family:ui-monospace,Consolas,monospace;font-size:11px}
.stres{padding:10px 14px;border-radius:10px;font-size:13px;background:color-mix(in oklch,var(--sc) 12%,transparent);border-left:4px solid var(--sc)}
.pins{display:flex;flex-direction:column;gap:8px}
.prow{display:grid;grid-template-columns:62px minmax(30px,90px) 62px 1fr;align-items:center;gap:10px;color:var(--sc)}
.prow b{padding:5px 0;border-radius:8px;border:1.5px solid currentColor;text-align:center;font-size:12px;background:color-mix(in oklch,currentColor 10%,transparent)}
.prow i{height:3px;border-radius:2px;background:currentColor}.prow.bad i{background:repeating-linear-gradient(90deg,currentColor 0 6px,transparent 6px 10px)}
.prow small{color:var(--text);font-size:12px}.prow.hd{color:var(--muted)}.prow.hd b{border:0;background:none;font-weight:600;padding:0}
@media(max-width:820px){.hst{grid-template-columns:repeat(2,minmax(0,1fr))}}
@media(max-width:600px){.wrap{padding:0 16px}.hero{padding:18px 16px}.pw b{font-size:44px}.card{padding:16px}.brand small{display:none}
 .mbars.n .mv{display:none}.dlab span:nth-child(even){visibility:hidden}.g4{grid-template-columns:repeat(2,minmax(0,1fr))}
 .prow{grid-template-columns:62px 1fr 62px}.prow small{grid-column:1/-1;margin:-4px 0 4px}.today{display:none}}
</style></head><body>
<div class="topbar"><i class="rb"></i></div>
<div class="wrap">
<header>
 <div class="brand"><div class="logo"><svg viewBox="0 0 24 24"><path d="M13 2 3 14h9l-1 8 10-12h-9l1-8z"/></svg></div>
  <div><strong>Licznik energii</strong><small id="hsub">HAN wireless M-Bus</small></div></div>
 <nav class="seg" id="tabs"><button data-t="pulpit">Pulpit</button><button data-t="ustawienia">Ustawienia</button><button data-t="diag">Diagnostyka</button></nav>
 <button class="icb" id="thb" title="Zmień motyw"></button>
</header>
<div class="chips" id="chips"></div>
<main>
<div class="note" id="note" hidden></div>
<!-- ================= PULPIT ================= -->
<div id="v-pulpit" style="display:flex;flex-direction:column;gap:16px">
<section class="hero" id="hero">
 <div class="fw"><div class="flow">
  <svg viewBox="0 0 600 280"><path d="M150 140H250" class="ft"/><path d="M150 140H250" class="fg"/><path d="M150 140H250" class="fl"/>
   <path d="M350 140H450" class="ft"/><path d="M350 140H450" class="fg"/><path d="M350 140H450" class="fl"/></svg>
  <div class="fb" id="fb1" style="left:33.3%"></div><div class="fb" id="fb2" style="left:66.7%"></div>
  <div class="node" style="left:16.7%;--nc:var(--v)"><div class="nc"><svg viewBox="0 0 24 24"><path d="M12 2v20M3 5h18M5 3v2M19 3v2M17 5l-5 6-5-6"/></svg></div>
   <div class="nl"><span>Sieć</span><b id="ngrid">–</b><small id="ngs">sieć</small></div></div>
  <div class="node" style="left:50%;--nc:var(--y)"><div class="nc"><svg viewBox="0 0 24 24"><path d="m12 14 4-4M3.34 19a10 10 0 1 1 17.32 0"/></svg></div>
   <div class="nl"><span>Licznik</span><b id="nmet">–</b><small id="nms">wM-Bus</small></div></div>
  <div class="node" style="left:83.3%;--nc:var(--b)"><div class="nc"><svg viewBox="0 0 24 24"><path d="M15 21v-8a1 1 0 0 0-1-1h-4a1 1 0 0 0-1 1v8M3 10a2 2 0 0 1 .709-1.528l7-5.999a2 2 0 0 1 2.582 0l7 5.999A2 2 0 0 1 21 10v9a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/></svg></div>
   <div class="nl"><span>Dom</span><b id="nhome">–</b><small id="nhs">zużycie</small></div></div>
 </div>
 <div class="tot" id="tot"></div></div>
 <div class="hr">
  <div class="ch"><span class="hk">Moc chwilowa · teraz</span>
   <div class="seg hseg" id="ivs"><button data-v="2">2 s</button><button data-v="5">5 s</button><button data-v="10">10 s</button><button data-v="30">30 s</button><button data-v="60">60 s</button></div></div>
  <div style="display:flex;flex-direction:column;gap:6px"><span class="dir" id="dir">–</span>
   <div class="pw"><b id="pv">–</b><span id="pu">W</span></div><span class="hsub" id="psub"></span></div>
  <div style="display:flex;flex-direction:column;gap:8px"><div class="gauge"><i id="gnow" style="width:0"></i><em id="gmax" style="display:none"></em></div>
   <div class="gl"><span id="gll"></span><span id="glr"></span></div></div>
  <div class="hst" id="hst"></div>
 </div>
</section>

<section class="card">
 <div class="ch"><div><h2>Moc w czasie</h2><div class="sub">średnia z 30 s · najedź na wykres, aby zobaczyć szczegóły</div></div>
  <div class="seg" id="rg"><button data-r="900">15 min</button><button data-r="3600">1 h</button><button data-r="10800">3 h</button><button data-r="21600">6 h</button></div></div>
 <div id="chart"></div>
 <div class="ch"><div class="leg"><span><i class="sw" style="background:var(--o)"></i>Pobór z sieci</span><span id="lgexp" hidden><i class="sw" style="background:var(--g)"></i>Oddawanie</span></div>
  <div style="display:flex;gap:8px" id="cbad"></div></div>
</section>

<section class="card">
 <div class="ch"><div><h2>Zużycie dzienne<span id="dname"></span></h2><div class="sub">najedź na słupek, aby zobaczyć wartość</div></div>
  <div style="display:flex;gap:6px"><button class="btn" id="dprev">‹ poprz.</button><button class="btn" id="dnext">nast. ›</button></div></div>
 <div class="stats" id="dstat"></div>
 <div id="dchart"></div>
 <div class="leg"><span><i class="sw" style="background:var(--o)"></i>powyżej średniej</span><span><i class="sw" style="background:color-mix(in oklch,var(--o) 40%,var(--card))"></i>poniżej średniej</span>
  <span><i style="width:14px;border-top:1.5px dashed var(--o)"></i>średnia <span id="davg">–</span></span><span><b style="color:var(--red)">So Nd</b> weekend</span><span>półprzezroczysty – szacunek lub w toku</span></div>
</section>

<section class="card" id="napiecia">
 <div class="ch"><div><h2>Napięcia faz</h2><div class="sub">norma 207–253 V · wykres od otwarcia strony (<span id="vspan">–</span>)</div></div>
  <span class="leg"><span><i style="width:18px;border-top:1.5px dashed var(--red)"></i>253 V – przy tej granicy falownik PV się wyłącza</span></span></div>
 <div class="phs" id="phs"></div>
 <div class="row">
  <div class="vch"><svg viewBox="0 0 600 160" preserveAspectRatio="none">
    <rect x="0" y="18.7" width="600" height="122.6" fill="color-mix(in oklch,var(--g) 9%,transparent)"/>
    <line x1="0" x2="600" y1="18.7" y2="18.7" stroke="var(--red)" stroke-dasharray="5 4" vector-effect="non-scaling-stroke"/>
    <line x1="0" x2="600" y1="141.3" y2="141.3" stroke="var(--red)" stroke-opacity=".5" stroke-dasharray="5 4" vector-effect="non-scaling-stroke"/>
    <line x1="0" x2="600" y1="80" y2="80" stroke="var(--bdr)" vector-effect="non-scaling-stroke"/><g id="vl"></g></svg>
   <span class="lb" style="top:21px;color:var(--red);font-weight:600">253</span><span class="lb" style="top:90px">230</span><span class="lb" style="top:159px">207</span>
   <div class="vempty" id="vempty">Wykres wypełnia się kolejnymi odczytami od momentu otwarcia strony.</div>
   <div class="ticks" id="vticks"></div></div>
  <div class="dvs"><div class="ch" style="padding-bottom:6px"><b style="font-size:14px">Odchylenia od normy</b>
    <span style="display:flex;gap:2px"><button class="btn gh" id="vxml">XML</button><button class="btn gh" id="vpdf">PDF</button></span></div>
   <div class="sub" id="vsum" style="padding-bottom:6px">–</div><div id="vtab"></div>
   <div class="vexp"><label>Od<input class="in" type="date" id="vfrom"></label><label>Do<input class="in" type="date" id="vto"></label>
    <form method="post" action="/voltclear" style="margin-left:auto" onsubmit="return confirm('Usunąć wszystkie zapisane odchylenia napięcia?')"><button class="btn del">Wyczyść</button></form></div></div>
 </div>
</section>

<section class="card">
 <div class="ch"><div><h2>Zestawienie miesięczne</h2><div class="sub">z dobowych stanów licznika zapisanych w ESP · kliknij miesiąc, aby zobaczyć szczegóły i dni</div></div>
  <div class="seg" id="mr"><button data-m="6">6 mies.</button><button data-m="12">12 mies.</button><button data-m="24">24 mies.</button></div></div>
 <div class="mbars" id="mbars"></div>
 <div class="ch"><div class="leg"><span><i class="sw" style="background:var(--o)"></i>Pobór</span><span><i class="sw" style="background:var(--o);opacity:.45"></i>okres w toku, niepełny lub szacowany</span></div>
  <span style="display:flex;gap:6px"><button class="btn gh" id="csvm">CSV · miesiące</button><button class="btn gh" id="csvd">CSV · dni</button></span></div>
 <div class="mdet"><div class="ch"><b id="mdn">–</b><span class="sub" id="mdnote"></span></div><div class="mi" id="mdi"></div></div>
</section>

<section class="card">
 <div class="ch"><div><h2>Energia wg stref<span id="zname"></span></h2><div class="sub" id="zsub">pobór z sieci</div></div><span class="pill" id="ztar" style="background:var(--track)"></span></div>
 <div style="display:flex;flex-wrap:wrap;gap:32px;align-items:center">
  <div class="donut"><svg viewBox="0 0 120 120" width="150" height="150"><circle cx="60" cy="60" r="46" fill="none" stroke="var(--track)" stroke-width="13"/><g id="dnt"></g></svg>
   <div><b id="dtot">–</b><span class="sub" id="dtu">kWh pobrane</span></div></div>
  <div class="tl" id="tl"></div></div>
 <div class="sub" id="expl"></div>
</section>
</div>

<!-- ================= USTAWIENIA ================= -->
<div id="v-ustawienia" hidden style="flex-direction:column;gap:16px">
 <div class="ttl"><h1>Ustawienia</h1><div class="sub" style="font-size:14px">Zapis wymaga hasła administratora (użytkownik <b>admin</b>).</div></div>
 <section class="grid">
  <div class="card"><form method="post" action="/config"><h3>Ceny energii</h3>
   <p class="sub" style="margin:0">Cena brutto za 1 kWh razem z dystrybucją (składniki zależne od zużycia). Puste T2–T4 = jak T1.</p>
   <div class="f"><label>Taryfa (licznik nie przesyła stref – ESP liczy je sam wg godzin TAURON)</label><select class="in" name="tariff" id="tar">
    <option value="0">G11 – całodobowa</option><option value="1">G12 – dzienna / nocna</option><option value="2">G12w – + weekendy</option><option value="3">G13 – trzy strefy</option></select></div>
   <div class="sub" id="tarh"></div>
   <div class="g4"><div class="f"><label id="pl1">T1</label><input class="in" name="p1" id="p1" inputmode="decimal" placeholder="zł/kWh"></div><div class="f"><label id="pl2">T2</label><input class="in" name="p2" id="p2" inputmode="decimal" placeholder="—"></div>
    <div class="f"><label id="pl3">T3</label><input class="in" name="p3" id="p3" inputmode="decimal" placeholder="—"></div><div class="f"><label>T4</label><input class="in" name="p4" id="p4" inputmode="decimal" placeholder="—"></div></div>
   <div class="f"><label>Opłaty stałe (abonament, opłaty sieciowe), zł/mies.</label><input class="in" name="fee" id="fee" inputmode="decimal"></div>
   <button class="btn pri mt-a">Zapisz ceny</button></form></div>
  <div class="card"><form method="post" action="/reading"><h3>Stan licznika z rachunku</h3>
   <p class="sub" style="margin:0">Pokazywany na wykresie zużycia dziennego jako pionowa kreska ze stanem i kwotą. Stany sprzed montażu czujnika uzupełniają też historię (zużycie rozkładane równo na dni).</p>
   <div class="g2"><div class="f"><label>Data odczytu</label><input class="in" type="date" name="date" required></div><div class="f"><label>Pobór, kWh</label><input class="in" name="imp" inputmode="decimal" required></div>
    <div class="f"><label>T1, kWh</label><input class="in" name="t1" inputmode="decimal" placeholder="opcjonalnie"></div><div class="f"><label>T2, kWh</label><input class="in" name="t2" inputmode="decimal" placeholder="opcjonalnie"></div>
    <div class="f"><label>Oddanie, kWh</label><input class="in" name="exp" inputmode="decimal" placeholder="opcjonalnie"></div>
    <div class="f"><label>Do zapłaty, zł</label><input class="in" name="amount" inputmode="decimal" placeholder="opcjonalnie"></div></div>
   <button class="btn mt-a">Dodaj stan</button></form><div id="manual"></div></div>
  <div class="card"><form method="post" action="/config"><h3>Licznik</h3>
   <div class="f"><label>Klucz AES (32 znaki hex, z TAURON eLicznik)</label><input class="in mono" name="key" placeholder="pozostaw puste, aby nie zmieniać" autocomplete="off"></div>
   <div class="f"><label>ID licznika (puste = dowolny)</label><input class="in" name="id" id="mid"></div>
   <button class="btn mt-a">Zapisz</button></form></div>
  <div class="card"><h3>Mini-cloud</h3>
   <div style="display:flex;align-items:center;gap:8px;flex-wrap:wrap"><span class="pill" id="cpill"></span><span class="sub" id="cst">–</span></div>
   <div id="capp" style="display:none;font-size:14px">Aplikacja Android — login <b id="cu"></b> · hasło <b id="cp">••••••</b> <button type="button" class="btn gh" style="color:var(--b);padding:2px 6px" id="cpb">Pokaż</button></div>
   <div style="display:flex;gap:8px;flex-wrap:wrap" class="mt-a">
    <form method="post" action="/cloudregister" id="creg" onsubmit="return confirm('Zarejestrować licznik w mini-cloud? Otrzymasz login i hasło do aplikacji.')"><button class="btn pri">Rejestruj w mini-cloud</button></form>
    <form method="post" action="/config" id="cdel" style="display:none" onsubmit="return confirm('Usunąć rejestrację? ESP przestanie wysyłać odczyty.')"><input type="hidden" name="cloud_forget" value="1"><button class="btn del">Usuń rejestrację</button></form></div></div>
  <div class="card"><form method="post" action="/config"><h3>Hasło administratora</h3>
   <div class="g2"><div class="f"><label>Nowe hasło (4–32 znaki)</label><input class="in" type="password" name="pass" autocomplete="new-password" required></div>
    <div class="f"><label>Powtórz hasło</label><input class="in" type="password" name="pass2" autocomplete="new-password" required></div></div>
   <p class="sub" style="margin:0">To samo hasło obowiązuje przy aktualizacji OTA — zmień też <code>--auth</code> w platformio.ini.</p>
   <button class="btn mt-a">Zmień hasło</button></form></div>
  <div class="card"><h3>Aktualizacja oprogramowania</h3>
   <p class="sub" style="margin:0">Z przeglądarki: plik <code>.pio/build/nodemcuv2/firmware.bin</code>. Przez Wi-Fi: <code>python -m platformio run -e ota -t upload</code>.</p>
   <a class="btn pri mt-a" href="/update">Wgraj firmware…</a></div>
 </section>
</div>

<!-- ================= DIAGNOSTYKA ================= -->
<div id="v-diag" hidden style="flex-direction:column;gap:16px">
 <div class="ttl"><h1>Diagnostyka</h1><div class="sub" style="font-size:14px">Radio CC1101 · odbiór ramek wM-Bus</div></div>
 <section class="grid">
  <div class="card"><h3>Test połączeń CC1101</h3>
   <p class="sub" style="margin:0">Sprawdza przewody NodeMCU ↔ moduł radiowy: zwarcia, brak odpowiedzi, komunikację SPI. Odbiór przerywany na ułamek sekundy.</p>
   <div id="stt" class="pins"></div><button class="btn mt-a" id="stb">Sprawdź połączenia</button></div>
  <div class="card"><h3>Radio</h3><div class="kv" id="rf"></div><div class="sub" id="rfn"></div></div>
  <div class="card"><h3>Licznik</h3><div class="kv" id="info"></div></div>
  <div class="card" style="grid-column:1/-1"><h3>Odebrane urządzenia</h3><div class="w"><table id="dev"></table></div></div>
  <div class="dark"><h3>Ostatnie rekordy</h3><pre id="rec">–</pre></div>
  <div class="dark"><h3>Ostatnio wysłane do mini-cloud</h3><pre id="cpl">–</pre></div>
 </section>
</div>
</main></div>
<script>
const $=i=>document.getElementById(i),qa=(s,e)=>[...(e||document).querySelectorAll(s)];
const esc=s=>String(s??'').replace(/[&<>"]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]));
const fx=(v,d=0)=>v==null||isNaN(v)?'–':v.toLocaleString('pl-PL',{minimumFractionDigits:d,maximumFractionDigits:d});
const kw=w=>w==null?'–':Math.abs(w)>=1000?fx(w/1000,1)+' kW':fx(Math.round(w))+' W';
const lg=k=>{try{return localStorage.getItem(k)}catch(e){return null}},ls=(k,d)=>+lg(k)||d,ss=(k,v)=>{try{localStorage.setItem(k,v)}catch(e){}};
function nice(v){const p=Math.pow(10,Math.floor(Math.log10(v))),n=v/p;return(n<=1?1:n<=2?2:n<=2.5?2.5:n<=5?5:10)*p}
const dur=s=>s<60?s+' s':s<3600?Math.floor(s/60)+' min':s<86400?Math.floor(s/3600)+' h '+Math.floor(s%3600/60)+' min':Math.floor(s/86400)+' d '+Math.floor(s%86400/3600)+' h';
const hhmm=t=>{const d=new Date(t);return d.getHours()+':'+String(d.getMinutes()).padStart(2,'0')};
const clamp=(v,a,b)=>Math.max(a,Math.min(b,v)),WD=['Nd','Pn','Wt','Śr','Cz','Pt','So'];
// ---- motyw ----
const SUN='<svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="4"/><path d="M12 2v2M12 20v2M4.93 4.93l1.41 1.41M17.66 17.66l1.41 1.41M2 12h2M20 12h2M6.34 17.66l-1.41 1.41M19.07 4.93l-1.41 1.41"/></svg>',
 MOON='<svg viewBox="0 0 24 24"><path d="M12 3a6 6 0 0 0 9 9 9 9 0 1 1-9-9Z"/></svg>';
let dark=lg('th')?lg('th')=='d':matchMedia('(prefers-color-scheme:dark)').matches;
function theme(){document.documentElement.dataset.th=dark?'dark':'light';$('thb').innerHTML=dark?SUN:MOON;$('mtc').content=dark?'#0a0d14':'#f2f3f6'}
$('thb').onclick=()=>{dark=!dark;ss('th',dark?'d':'l');theme()};theme();
// ---- zakladki ----
let tab='pulpit',stRun=0;
function go(t){tab=t;['pulpit','ustawienia','diag'].forEach(k=>{const e=$('v-'+k);e.hidden=k!=t;e.style.display=k==t?'flex':'none'});
 qa('button',$('tabs')).forEach(b=>b.classList.toggle('on',b.dataset.t==t));
 history.replaceState(null,'',t=='pulpit'?location.pathname:'#'+t);
 if(t=='diag'&&!stRun)selftest();if(t=='pulpit'){draw();drawMonths()}}
$('tabs').onclick=e=>{if(e.target.dataset.t)go(e.target.dataset.t)};
const segOn=(id,k,v)=>qa('button',$(id)).forEach(b=>b.classList.toggle('on',+b.dataset[k]==v));
// ---- odswiezanie ----
let ivs=ls('iv',5),range=ls('rg',3600),H=[],hLast=0,hSync=0,up=0,J=null,tmr,next=0,loadAt=0;
$('ivs').onclick=e=>{if(e.target.dataset.v){ivs=+e.target.dataset.v;ss('iv',ivs);segOn('ivs','v',ivs);sched(0)}};
function setRange(r){range=r;ss('rg',r);segOn('rg','r',r);draw()}
$('rg').onclick=e=>{if(e.target.dataset.r)setRange(+e.target.dataset.r)};
function sched(ms){clearTimeout(tmr);next=Date.now()+ms;tmr=setTimeout(tick,ms)}
async function tick(){await load();sched(ivs*1000)}
(function pb(){const w=clamp(100-(next-Date.now())/(ivs*10),0,100)+'%';qa('.rb').forEach(e=>e.style.width=w);requestAnimationFrame(pb)})();
setInterval(()=>{const a=$('ago'),m=J&&J.meter;if(a&&m){const s=Math.round(m.age_s+(Date.now()-loadAt)/1000);a.textContent=s<2?'teraz':dur(s)+' temu'}},1000);
let skew=0;const wall=t=>t*1000+skew;
async function load(){
 try{const j=await(await fetch('/json',{cache:'no-store'})).json();
  if(j.uptime_s<up){H=[];hLast=0;hSync=0}up=j.uptime_s;J=j;loadAt=Date.now();if(j.time)skew=Date.now()-j.time*1000;
  if(Date.now()-hSync>25000){try{const h=await(await fetch('/history?since='+hLast,{cache:'no-store'})).json();
   h.s.forEach(s=>H.push({t:s[0],i:s[1],e:s[2]}));if(H.length)hLast=H[H.length-1].t;
   H=H.filter(p=>p.t>(j.time||1e10)-22000);hSync=Date.now()}catch(e){}}
  const last=D.length?D[D.length-1][0]:0;
  if(Date.now()-dSync>(last==todayN()?600000:60000)){try{const dj=await(await fetch('/days',{cache:'no-store'})).json();D=dj.d;R=dj.r||[];dSync=Date.now();mDrawn=0}catch(e){}}
  if(Date.now()-vSync>60000)await loadVolt();
  vpush(j);render(j)}catch(e){console.error(e);offline()}}
const DOT={ok:'g',warn:'amb',bad:'red'},chip=(s,t)=>`<span class="chip"><span class="dot" style="background:var(--${DOT[s]||'muted'})"></span>${t}</span>`,
 today=()=>`<span class="today">${new Date().toLocaleDateString('pl-PL',{weekday:'long',day:'numeric',month:'long'})}</span>`;
function offline(){$('chips').innerHTML=chip('bad','<b>Brak połączenia z ESP</b>')+today()}
function chips(j){
 const m=j.meter,a=m?m.age_s:null,r=m?m.rssi:null,lv=r==null?0:r>-70?4:r>-80?3:r>-90?2:1;
 $('chips').innerHTML=`<span class="chip"><span class="mini"><i class="rb"></i></span>Odczyt <b id="ago">${a==null?'brak':a<2?'teraz':dur(a)+' temu'}</b><span>· co ${ivs} s</span></span>`+
  chip(a==null?'warn':a<120?'ok':a<600?'warn':'bad',a==null?'<b>Czekam na odczyt</b>':'Licznik <b>'+(a<120?'nadaje':'cisza')+'</b>')+
  chip(j.radio_ok?'ok':'bad','Radio CC1101 <b>'+(j.radio_ok?'OK':'brak')+'</b>')+chip(j.key_set?'ok':'bad','Klucz AES <b>'+(j.key_set?'ustawiony':'brak')+'</b>')+
  (m?`<span class="chip"><span class="sig">${[3,5,7.5,10].map((h,i)=>`<i style="height:${h}px" class="${i<lv?'on':''}"></i>`).join('')}</span>Sygnał <b>${r} dBm</b></span>`:'')+
  `<span class="chip">Ramki <b>${fx(j.frames)}</b> · CRC <b>${fx(j.crc_errors)}</b></span>`+today()}
function render(j){
 chips(j);const m=j.meter||{};
 $('hsub').innerHTML=j.meter?'Licznik <b>'+esc(m.serial||m.id)+'</b>'+(m.manufacturer?' · '+esc(m.manufacturer):'')+' · HAN wM-Bus':'HAN wireless M-Bus';
 let n='';if(!j.radio_ok)n='Nie wykryto modułu CC1101 – sprawdź połączenia SPI i zasilanie 3,3 V. <a href="#diag" onclick="go(\'diag\');return false">Uruchom test połączeń</a>';
 else if(!j.meter)n=j.key_set?'Czekam na pierwszy poprawny odczyt z licznika…':'Wpisz klucz AES w zakładce <a href="#ustawienia" onclick="go(\'ustawienia\');return false">Ustawienia</a>.';
 if(m.message)n+=(n?' ':'')+'Komunikat licznika: '+esc(m.message);
 $('note').hidden=!n;$('note').innerHTML=n;
 calc();hero(j);volt(j);info(j);rfStats(j);settings(j);
 if(tab=='pulpit'){draw();if(Date.now()-mDrawn>30000)drawMonths()}}
// ---- karta glowna ----
function hero(j){
 const m=j.meter||{},imp=m.power_import_w,ex=m.power_export_w,gen=ex>0&&!(imp>0),v=gen?ex:imp,pr=j.price&&j.price[0]>0?j.price[0]:null,on=v!=null&&v>40;
 $('hero').classList.toggle('gen',gen);
 qa('.fl,.fg,.fb',$('hero')).forEach(e=>e.style.opacity=on?1:0);
 qa('.fl',$('hero')).forEach(e=>e.style.animationDuration=on?Math.max(.5,3-v/2200).toFixed(2)+'s':'1s');
 $('fb1').textContent=kw(v);$('fb2').textContent=!gen&&pr&&v!=null?'≈ '+fx(v/1000*pr,2)+' zł/h':kw(v);
 const vs=[1,2,3].map(p=>m['voltage_l'+p]).filter(x=>x!=null);
 $('ngrid').textContent=vs.length?fx(vs.reduce((a,b)=>a+b)/vs.length,0)+' V':'–';$('ngs').textContent=vs.length?vs.length+(vs.length>1?' fazy':' faza'):'sieć';
 $('nmet').textContent=m.serial||m.id||'–';$('nms').textContent=(m.manufacturer?m.manufacturer+' · ':'')+'wM-Bus';
 $('nhome').textContent=kw(v);
 const K=(l,x)=>{const[a,b]=fx(x,1).split(',');return`<div><span>${l}</span><b>${a}<small>,${b} kWh</small></b></div>`};
 $('tot').innerHTML=m.import_kwh==null?'<div><span>Stan licznika</span><b>–</b></div>':K('Pobrane łącznie',m.import_kwh)+(m.export_kwh>0?'<i></i>'+K('Oddane łącznie',m.export_kwh):'');$('nhs').textContent=gen?'nadwyżka PV':'zużycie';
 $('dir').textContent=v==null?'Brak odczytu':gen?'↑ Oddajesz do sieci':'↓ Pobór z sieci';
 $('pv').textContent=v==null?'–':v>=10000?fx(v/1000,1):fx(Math.round(v));$('pu').textContent=v>=10000?'kW':'W';
 $('psub').textContent=v==null?'czekam na odczyt z licznika':gen?'nadwyżka z fotowoltaiki trafia do sieci':pr?'≈ '+fx(v/1000*pr,2)+' zł/h przy cenie '+fx(pr,2)+' zł/kWh':'ustaw cenę energii, aby zobaczyć koszt';
 const lim=m.power_limit_w>0?m.power_limit_w:null,mx=m.today_max_w!=null?m.today_max_w:m.power_max_w,thr=lim||nice(Math.max(5000,mx||0,v||0));
 $('gnow').style.width=clamp((v||0)/thr*100,0,100)+'%';$('gmax').style.display=mx>0?'':'none';$('gmax').style.left=clamp(mx/thr*100,0,100)+'%';
 $('gll').textContent=lim?Math.round((v||0)/lim*100)+'% progu mocy '+kw(lim):'skala '+kw(thr);$('glr').textContent=mx>0?'maks. dziś '+kw(mx):'';
 const n=new Date(),ti=dnum(n.getFullYear(),n.getMonth(),n.getDate()),td=DAYS[ti],yd=DAYS[ti-1],ck=n.getFullYear()*100+n.getMonth()+1,
  c=MON.find(q=>q.key==ck),pv=MON.find(q=>q.key==(ck%100==1?ck-89:ck-1)),fee=j.fee_month||0,
  tI=m.today_import_kwh!=null?m.today_import_kwh:td&&td.v[0]!=null?td.v[0]/1000:null,
  avg=c&&c.v[0]!=null&&c.cov>0?c.v[0]/c.cov/1000:null,mI=m.month_import_kwh!=null?m.month_import_kwh:c&&c.v[0]!=null?c.v[0]/1000:null,
  pI=pv&&pv.v[0]!=null?pv.v[0]/1000:null,mC=m.month_cost_pln!=null?m.month_cost_pln+fee:c?mc(c):null,
  aC=c&&c.cost!=null&&c.cov>0?c.cost/c.cov:null,exp=m.month_export_kwh>0||m.today_export_kwh>0;
 const T=(k,v,u,p,g,nt)=>`<div class="hs"><span>${k}</span><div><b>${v}</b><u>${u}</u></div><div class="hb"><i style="width:${clamp((p||0)*100,0,100).toFixed(0)}%;background:linear-gradient(90deg,${g})"></i></div><small>${nt}</small></div>`;
 $('hst').innerHTML=T('Pobrano dziś',fx(tI,1),'kWh',avg&&tI/avg,'var(--y),var(--o)',avg!=null?'średnio '+fx(avg,1)+' kWh / dzień':yd&&yd.v[0]!=null?'wczoraj '+kwh(yd.v[0],1)+' kWh':'&nbsp;')+
  T('Ten miesiąc',fx(mI,0),'kWh',pI&&mI/pI,'var(--t),var(--b)',pI!=null?'poprz. miesiąc '+fx(pI,0)+' kWh':'&nbsp;')+
  (exp?T('Oddano dziś',fx(m.today_export_kwh,1),'kWh',m.month_export_kwh>0&&m.today_export_kwh/(m.month_export_kwh/n.getDate()),'var(--t),var(--g)','w miesiącu '+fx(m.month_export_kwh,0)+' kWh')
   :T('Moc maks. dziś',mx!=null?fx(mx/1000,1):'–','kW',mx/thr,'var(--o),var(--red)',m.today_max_at?'o '+hhmm(m.today_max_at*1000)+(lim?' · próg '+kw(lim):''):lim?'próg mocy '+kw(lim):'&nbsp;'))+
  T('Koszt dziś',td&&td.cost!=null?fx(td.cost,2):'–','zł',aC&&td&&td.cost/aC,'var(--b),var(--v)',mC!=null?'miesiąc ≈ '+fx(mC,0)+' zł':'ustaw ceny w Ustawieniach')}
// ---- wykres mocy ----
let P=[],X,Yi,t0;
function draw(){
 const el=$('chart'),W=el.clientWidth||600,Hh=el.clientHeight||230,L=52,R=8,T=10,B=24,now=Date.now();t0=now-range*1000;
 P=H.map(p=>({x:wall(p.t),i:p.i,e:p.e}));const m=J&&J.meter;
 if(m&&m.age_s<120&&J.time){const x=wall(J.time-m.age_s);if(!P.length||x>P[P.length-1].x+1000)P.push({x,i:m.power_import_w,e:m.power_export_w})}
 P=P.filter(p=>p.x>=t0-60000);
 const vi=P.map(p=>p.i).filter(v=>v!=null),ve=P.map(p=>p.e).filter(v=>v!=null),hasE=ve.some(v=>v>0);
 $('lgexp').hidden=!hasE;
 $('cbad').innerHTML=(vi.length?`<span class="badge" style="--k:var(--o)">maks. pobór ${kw(Math.max(...vi))}</span>`:'')+(hasE?`<span class="badge" style="--k:var(--g)">maks. oddawanie ${kw(Math.max(...ve))}</span>`:'');
 if(!P.length){el.innerHTML='<div class="empty" style="height:100%">Brak danych z tego okresu.<br>Wykres wypełnia się automatycznie (próbka co 30 s).</div>';return}
 const mx=nice(Math.max(100,...P.map(p=>Math.max(p.i||0,p.e||0)))*1.1);
 X=x=>L+(x-t0)/(range*1000)*(W-L-R);Yi=v=>T+(1-v/mx)*(Hh-T-B);const base=Yi(0);
 const segs=k=>{const out=[];let c=[];P.forEach((p,n)=>{const gap=n&&p.x-P[n-1].x>150000;if(p[k]==null||gap){if(c.length)out.push(c);c=[]}if(p[k]!=null)c.push([X(p.x),Yi(p[k])])});if(c.length)out.push(c);return out};
 const ln=s=>s.map(c=>'M'+c.map(q=>q[0].toFixed(1)+','+q[1].toFixed(1)).join('L')).join('');
 const ar=s=>s.map(c=>c.length>1?'M'+c[0][0].toFixed(1)+','+base+'L'+c.map(q=>q[0].toFixed(1)+','+q[1].toFixed(1)).join('L')+'L'+c[c.length-1][0].toFixed(1)+','+base+'Z':'').join('');
 const si=segs('i'),se=segs('e');let g='';
 for(let k=0;k<=4;k++){const v=mx*k/4,y=Yi(v);g+=`<line x1="${L}" x2="${W-R}" y1="${y}" y2="${y}" stroke="var(--bdr)"/><text x="${L-8}" y="${y+4}" text-anchor="end">${v==0?'0':mx>=2000?fx(v/1000,v%1000?(v%100?2:1):0)+' kW':fx(v)+' W'}</text>`}
 const st=range<=900?300:range<=3600?900:range<=10800?1800:3600;
 for(let t=Math.ceil(t0/1000/st)*st*1000;t<=now;t+=st*1000){const x=X(t);g+=`<text x="${x}" y="${Hh-4}" text-anchor="middle">${hhmm(t)}</text>`}
 el.innerHTML=`<svg viewBox="0 0 ${W} ${Hh}"><defs><linearGradient id="gi" x1="0" x2="0" y1="0" y2="1"><stop offset="0" stop-color="var(--o)" stop-opacity=".55"/><stop offset="1" stop-color="var(--y)" stop-opacity=".04"/></linearGradient>
<linearGradient id="ge" x1="0" x2="0" y1="0" y2="1"><stop offset="0" stop-color="var(--g)" stop-opacity=".45"/><stop offset="1" stop-color="var(--t)" stop-opacity=".03"/></linearGradient></defs>${g}
<path d="${ar(si)}" fill="url(#gi)"/><path d="${ln(si)}" fill="none" stroke="var(--o)" stroke-width="2.25" stroke-linejoin="round"/>
${hasE?`<path d="${ar(se)}" fill="url(#ge)"/><path d="${ln(se)}" fill="none" stroke="var(--g)" stroke-width="2.25" stroke-linejoin="round"/>`:''}
<line x1="${L}" x2="${W-R}" y1="${base}" y2="${base}" stroke="var(--muted)" stroke-opacity=".6"/>
<g id="hv" style="display:none"><line id="hl" y1="${T}" y2="${base}" stroke="var(--muted)" stroke-opacity=".5"/><circle id="hi" r="6" fill="var(--o)" stroke="var(--card)" stroke-width="3"/><circle id="he" r="6" fill="var(--g)" stroke="var(--card)" stroke-width="3"/></g></svg><div class="tip" id="tip"></div>`}
function hover(ev){
 if(!P.length||!X)return;const el=$('chart'),r=el.getBoundingClientRect(),cx=ev.clientX-r.left;
 let b=P[0];P.forEach(p=>{if(Math.abs(X(p.x)-cx)<Math.abs(X(b.x)-cx))b=p});
 const x=X(b.x),hv=$('hv'),tip=$('tip');if(!hv)return;hv.style.display='';$('hl').setAttribute('x1',x);$('hl').setAttribute('x2',x);
 [['hi','i'],['he','e']].forEach(([id,k])=>{const c=$(id);const s=b[k]!=null&&(k=='i'||b[k]>0);c.style.display=s?'':'none';if(s){c.setAttribute('cx',x);c.setAttribute('cy',Yi(b[k]))}});
 const ge=b.e>0&&!(b.i>0);
 tip.innerHTML=`<span style="opacity:.65">${new Date(b.x).toLocaleTimeString('pl-PL')}</span><br><b style="font-size:18px">${kw(ge?b.e:b.i)}</b><br><b style="color:var(--${ge?'g':'o'})">${ge?'oddawanie do sieci':'pobór z sieci'}</b>`+(b.e>0&&b.i>0?'<br>oddawanie '+kw(b.e):'');
 tip.style.display='block';tip.style.left=(x>r.width*.65?x-tip.offsetWidth-14:x+14)+'px';tip.style.top='8px'}
$('chart').addEventListener('pointermove',hover);$('chart').addEventListener('pointerdown',hover);
$('chart').addEventListener('pointerleave',()=>{const hv=$('hv');if(hv)hv.style.display='none';if($('tip'))$('tip').style.display='none'});
// ---- dobowe stany: /days + biezacy odczyt ----
const MN=['sty','lut','mar','kwi','maj','cze','lip','sie','wrz','paź','lis','gru'],
 MF=['styczeń','luty','marzec','kwiecień','maj','czerwiec','lipiec','sierpień','wrzesień','październik','listopad','grudzień'];
const dnum=(y,m,d)=>Math.round(Date.UTC(y,m,d)/864e5),dof=n=>new Date(n*864e5),dimOf=k=>new Date(Date.UTC(Math.floor(k/100),k%100,0)).getUTCDate();
const todayN=()=>{const n=new Date();return n.getFullYear()*1e4+(n.getMonth()+1)*100+n.getDate()};
const mlab=(k,l)=>l?MF[k%100-1]+' '+Math.floor(k/100):MN[k%100-1];
const kwh=(v,d=1)=>v==null?'–':fx(v/1000,d);
let D=[],R=[],dSync=0,mRange=ls('mr',12),selM=0,mDrawn=0,DAYS={},MON=[];
// Zuzycie doby = roznica kolejnych stanow; przy przerwie w odczytach rozkladane rowno na dni (szacunek).
// Odczyty z rachunku (R) wchodza do historii tylko sprzed pierwszego automatycznego stanu - pozniejsze sa tylko znacznikami.
const rk=n=>dnum(Math.floor(n/1e4),Math.floor(n/100)%100-1,n%100);
function calc(){
 const f0=D.length?D[0][0]:1e9,
  p=[...R.filter(r=>r[0]<f0).map(r=>({k:rk(r[0]),v:[r[1],r[2],r[3],null,null,r[4]]})),...D.map(r=>({k:rk(r[0]),v:r.slice(1)}))],m=J&&J.meter;
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
// ---- zuzycie dzienne wybranego miesiaca ----
function dayChart(){
 const i=MON.findIndex(q=>q.key==selM);$('dprev').disabled=i<=0;$('dnext').disabled=i<0||i>=MON.length-1;
 $('dname').textContent=selM?' · '+mlab(selM,1):'';
 const y=Math.floor(selM/100),mo=selM%100,it=[];
 if(selM)for(let d=1;d<=dimOf(selM);d++){const o=DAYS[dnum(y,mo-1,d)];it.push({d,o,a:o&&o.v[0]!=null?o.v[0]/1000:null,wd:new Date(y,mo-1,d).getDay(),rd:R.find(r=>r[0]==selM*100+d)})}
 const dd=it.filter(x=>x.a!=null),full=dd.filter(x=>!x.o.live),ref=full.length?full:dd;
 if(!dd.length){$('dstat').innerHTML='';$('davg').textContent='–';$('dchart').innerHTML='<div class="empty">Brak danych.<br>Zestawienie tworzy się z dobowych stanów licznika zapisywanych w pamięci ESP.</div>';return}
 const mx=Math.max(...dd.map(x=>x.a)),hi=dd.reduce((a,x)=>x.a>a.a?x:a),lo=ref.reduce((a,x)=>x.a<a.a?x:a),tot=dd.reduce((s,x)=>s+x.a,0),
  avg=ref.reduce((s,x)=>s+x.a,0)/ref.length,top=nice(Math.max(mx,.1)*1.05),cs=dd.filter(x=>x.o.cost!=null),
  f=v=>fx(v,v<10&&v%1?1:0),S=(k,v)=>`<div class="st"><span>${k}</span><b>${v}</b></div>`;
 $('dstat').innerHTML=S('Razem',fx(tot,1)+' kWh'+(cs.length?' · ≈ '+fx(cs.reduce((s,x)=>s+x.o.cost,0),0)+' zł':''))+S('Średnio / dzień',fx(avg,1)+' kWh')+
  S('Najwięcej',fx(hi.a,1)+' kWh · '+hi.d+'.')+S('Najmniej',fx(lo.a,1)+' kWh · '+lo.d+'.');
 $('davg').textContent=fx(avg,1)+' kWh';
 $('dchart').innerHTML=`<div class="dc">${[top,top/2,0].map(v=>`<div class="gline" style="top:${22+100-v/top*100}px"></div><span class="glab" style="top:${22+100-v/top*100}px">${f(v)}</span>`).join('')}
<div class="avgl" style="top:${(22+100-avg/top*100).toFixed(0)}px"></div>
<div class="dbars">${it.map(x=>{const h=x.a!=null?Math.max(2,Math.round(x.a/top*100)):0;
 return`<div class="db">${x.rd?`<b class="rd${x.d>dimOf(selM)/2?' rt':''}"><span>Odczyt ${rdTxt(x.rd)}</span></b>`:''}<i class="${x.a>avg?'hi':''}${x.o&&(x.o.est||x.o.live)?' dm':''}" style="height:${h}px"></i><span class="tt" style="bottom:${h+6}px">${WD[x.wd]} ${x.d} · ${x.a!=null?fx(x.a,1)+' kWh'+(x.o.cost!=null?' · '+zl(x.o.cost):'')+(x.o.live?' (w toku)':x.o.est?' (szac.)':''):'brak danych'}${x.rd?'<br>Odczyt licznika: '+rdTxt(x.rd):''}</span></div>`}).join('')}</div>
<div class="dlab">${it.map(x=>`<span class="${x.wd==0||x.wd==6?'we':''}">${x.d}</span>`).join('')}</div></div>`}
// ---- zestawienie miesieczne ----
function monthBars(){
 const sh=MON.slice(-mRange);$('mbars').classList.toggle('n',sh.length>12);
 if(!sh.length){$('mbars').innerHTML='<div class="empty" style="flex:1">Brak danych.<br>Zestawienie tworzy się z dobowych stanów licznika zapisywanych w pamięci ESP.</div>';return}
 const mx=Math.max(1,...sh.map(q=>q.v[0]||0));
 $('mbars').innerHTML=sh.map((q,j)=>{const dim=q.live||q.est||q.n<dimOf(q.key);
  return`<button data-k="${q.key}" class="${q.key==selM?'on':''}" title="${mlab(q.key,1)}: ${kwh(q.v[0],0)} kWh"><div class="mb"><span class="mv">${kwh(q.v[0],0)} kWh</span><i style="height:${Math.round((q.v[0]||0)/mx*120)}px${dim?';opacity:.45':''}"></i></div><div class="ml">${mlab(q.key)+(q.key%100==1||j==0?" '"+String(q.key).slice(2,4):'')}</div></button>`}).join('')}
function mdet(){
 const q=MON.find(x=>x.key==selM);if(!q){$('mdn').textContent='–';$('mdnote').textContent='';$('mdi').innerHTML='';return}
 const dm=dimOf(q.key),pk=q.key%100==1?q.key-89:q.key-1,pv=MON.find(x=>x.key==pk),ly=MON.find(x=>x.key==q.key-100),
  pd=o=>o&&o.v[0]!=null&&o.cov>0?o.v[0]/o.cov:null,cur=pd(q),pct=(a,b)=>{const p=Math.round((a/b-1)*100);return(p>0?'+':'')+p+'%'},
  pc=(a,b)=>a>b?'var(--red)':'var(--okt)',I=(k,v,c)=>`<div><span>${k}</span><b${c?` style="color:${c}"`:''}>${v}</b></div>`;
 $('mdn').textContent=mlab(q.key,1);
 $('mdnote').textContent=q.live?'okres w toku – dane niepełne':(q.est?'szacunek · ':'')+q.n+' z '+dm+' dni z danymi';
 let h=I('Zużycie',(q.est?'≈ ':'')+kwh(q.v[0],0)+' kWh')+(q.cost!=null?I('Koszt ≈',fx(mc(q),0)+' zł'):'')+I('Średnio / dzień',kwh(cur,1)+' kWh');
 if(q.live&&cur!=null)h+=I('Prognoza na miesiąc','≈ '+kwh(cur*dm,0)+' kWh')+(q.cost!=null?I('Prognoza kosztu','≈ '+fx(q.cost/q.cov*dm+(J&&J.fee_month||0),0)+' zł'):'');
 h+=I('vs poprzedni miesiąc',pv&&pd(pv)?pct(cur,pd(pv)):'—',pv&&pd(pv)?pc(cur,pd(pv)):'var(--muted)')+I('vs rok temu',ly&&pd(ly)?pct(cur,pd(ly)):'brak danych',ly&&pd(ly)?pc(cur,pd(ly)):'var(--muted)');
 const tt=[1,2,3,4].filter(c=>q.v[c]>0);if(tt.length>1)tt.forEach(c=>h+=I(zn(c),kwh(q.v[c],0)+' kWh'));
 if(q.v[5]>0)h+=I('Oddane',kwh(q.v[5],0)+' kWh')+I('Bilans',kwh(q.v[0]-q.v[5],0)+' kWh');
 $('mdi').innerHTML=h}
const rdTxt=r=>kwh(r[1],0)+' kWh'+(r[5]!=null?' · do zapłaty '+zl(r[5]/100):'');
function manualList(){
 const ds=n=>String(n).replace(/(\d{4})(\d\d)(\d\d)/,'$1-$2-$3');
 $('manual').innerHTML=R.slice().reverse().map(r=>`<form method="post" action="/reading" class="mrow" onsubmit="return confirm('Usunąć stan z ${ds(r[0])}?')"><input type="hidden" name="date" value="${ds(r[0])}"><input type="hidden" name="del" value="1">
  <span>${ds(r[0])} · <b>${kwh(r[1],3)} kWh</b>${r[4]!=null?' · oddanie '+kwh(r[4],3)+' kWh':''}${r[5]!=null?' · <b>'+zl(r[5]/100)+'</b>':''}</span><button class="btn del" style="padding:3px 10px">Usuń</button></form>`).join('')}
function drawMonths(){
 mDrawn=Date.now();calc();manualList();
 if(!MON.some(q=>q.key==selM))selM=MON.length?MON[MON.length-1].key:0;
 monthBars();mdet();dayChart();if(J)tariffs(J.meter||{})}
function setMR(r){mRange=r;ss('mr',r);segOn('mr','m',r);drawMonths()}
$('mr').onclick=e=>{if(e.target.dataset.m)setMR(+e.target.dataset.m)};
$('mbars').onclick=e=>{const b=e.target.closest('button[data-k]');if(b){selM=+b.dataset.k;drawMonths()}};
$('dprev').onclick=()=>{const i=MON.findIndex(q=>q.key==selM);if(i>0){selM=MON[i-1].key;drawMonths()}};
$('dnext').onclick=()=>{const i=MON.findIndex(q=>q.key==selM);if(i>=0&&i<MON.length-1){selM=MON[i+1].key;drawMonths()}};
// ---- strefy taryfowe ----
const TC=['var(--o)','var(--v)','var(--b)','var(--y)'],
 ZN={1:['dzienna','nocna'],2:['dzienna','nocna i weekend'],3:['szczyt przedpołudniowy','szczyt popołudniowy','pozostałe godziny']},
 ZH={0:'Jedna cena przez całą dobę.',1:'T1: 6–13 i 15–22 · T2: 13–15 i 22–6 (codziennie).',2:'Dni robocze jak G12 · soboty, niedziele i święta w całości T2.',
  3:'Dni robocze: T1 7–13 · T2 19–22 (kwi–wrz) lub 16–21 (paź–mar) · T3 pozostałe godziny. Soboty, niedziele i święta w całości T3.'},
 TN=['G11','G12','G12w','G13'],zn=c=>'T'+c+((ZN[J&&J.tariff]||[])[c-1]?' · '+ZN[J.tariff][c-1]:'');
function tarLabels(){const v=+$('tar').value;$('tarh').textContent=ZH[v];[1,2,3].forEach(c=>$('pl'+c).textContent='T'+c+(ZN[v]&&ZN[v][c-1]?' · '+ZN[v][c-1]:''))}
$('tar').onchange=tarLabels;
function tariffs(m){
 // podzial wybranego miesiaca (z dobowych stanow), a gdy go brak - liczniki stref od poczatku liczenia
 const q=MON.find(x=>x.key==selM),tf=J&&J.tariff||0;let t=[],tot=null,lab='';
 if(q&&[1,2,3,4].some(c=>q.v[c]>0)){t=[1,2,3,4].map(c=>({k:c,l:zn(c),v:q.v[c]/1000,c:TC[c-1]})).filter(x=>x.v>0);tot=t.reduce((a,x)=>a+x.v,0);lab=' · '+mlab(q.key,1)}
 else{t=[1,2,3,4].map(c=>({k:c,l:zn(c),v:m['import_t'+c+'_kwh'],c:TC[c-1]})).filter(x=>x.v>0);tot=t.length?t.reduce((a,x)=>a+x.v,0):m.import_kwh;
  if(t.length&&J.zones_since)lab=' · od '+String(J.zones_since).replace(/(\d{4})(\d\d)(\d\d)/,'$3.$2.$1')}
 $('zname').textContent=lab;$('ztar').textContent=TN[tf];$('zsub').textContent=ZH[tf];
 if(!t.length){$('dnt').innerHTML='';$('dtot').textContent=tot!=null?fx(tot,0):'–';$('dtu').textContent='kWh (stan licznika)';
  $('tl').innerHTML='<span class="sub">'+(tf?'Strefy liczone od teraz – pierwsze wartości pojawią się po kolejnym odczycie licznika.':'Licznik nie przesyła podziału na strefy. Wybierz swoją taryfę (np. G13) w <a href="#ustawienia" onclick="go(\'ustawienia\');return false">Ustawieniach</a> – ESP będzie liczyć strefy samo.')+'</span>';$('expl').innerHTML='';return}
 $('dtu').textContent='kWh pobrane';
 const s=t.reduce((a,x)=>a+x.v,0),pm=Math.max(0,...t.map(x=>x.v/s)),gap=t.length>1;let acc=0;
 $('dtot').textContent=fx(tot,tot<100?1:0);
 $('dnt').innerHTML=t.map(x=>{const p=x.v/s,o=`<circle cx="60" cy="60" r="46" fill="none" stroke="${x.c}" stroke-width="13" stroke-linecap="${gap?'round':'butt'}" pathLength="100" stroke-dasharray="${Math.max(0,p*100-(gap?4:0)).toFixed(2)} 100" stroke-dashoffset="${(-acc*100-(gap?2:0)).toFixed(2)}"/>`;acc+=p;return o}).join('');
 $('tl').innerHTML=t.length?t.map(x=>`<div><div class="ch"><span><i class="sw" style="background:${x.c}"></i>${x.l}</span><span><b>${fx(x.v,1)} kWh</b> <span class="sub">· ${fx(x.v/s*100)}%${J&&J.price&&J.price[0]>0?' · ≈ '+fx(x.v*(J.price[x.k-1]||J.price[0]),2)+' zł':''}</span></span></div><div class="bar"><i style="width:${(x.v/s/pm*100).toFixed(0)}%;background:${x.c}"></i></div></div>`).join(''):'<span class="sub">Brak danych z licznika.</span>';
 const et=[1,2,3,4].map(c=>[c,m['export_t'+c+'_kwh']]).filter(x=>x[1]>0);
 $('expl').innerHTML=m.export_kwh>0?'Oddane do sieci (stan): <b style="color:var(--text)">'+fx(m.export_kwh,1)+' kWh</b>'+(et.length>1?' · '+et.map(x=>'T'+x[0]+' '+fx(x[1],1)).join(' · '):''):''}
function info(j){
 const m=j.meter||{},inf=[['Numer seryjny',m.serial],['ID radiowe',m.id],['Producent',m.manufacturer],['Czas licznika',m.datetime],
  ['Ostatni odczyt',m.age_s!=null?dur(m.age_s)+' temu':null],['Sygnał',m.rssi!=null?m.rssi+' dBm':null],['Czas pracy ESP',dur(j.uptime_s)],['Filtr ID',j.meter_filter||'brak'],
  ['Zapisane doby',j.days_stored==null?null:j.days_stored<0?'błąd pamięci':String(j.days_stored)]];
 $('info').innerHTML=inf.filter(x=>x[1]).map(x=>`<div><span>${x[0]}</span><b>${esc(x[1])}</b></div>`).join('')}
// ---- napiecia: bufor z kolejnych odczytow (od otwarcia strony) ----
let VB=[];const PC=['var(--b)','var(--p2)','var(--p3)'];
function vpush(j){const m=j.meter;if(!m||m.voltage_l1==null&&m.voltage_l2==null&&m.voltage_l3==null)return;
 const x=Date.now()-m.age_s*1000;if(VB.length&&x-VB[VB.length-1].x<1500)return;
 VB.push({x,v:[m.voltage_l1,m.voltage_l2,m.voltage_l3]});if(VB.length>2000)VB.shift()}
function volt(j){
 const m=j.meter||{},vp=v=>clamp((v-200)/60*100,0,100);
 const ph=[0,1,2].map(i=>({i,v:m['voltage_l'+(i+1)]})).filter(x=>x.v!=null);
 $('phs').innerHTML=ph.length?ph.map(({i,v})=>{const a=VB.map(b=>b.v[i]).filter(x=>x!=null),mn=Math.min(v,...a),mx=Math.max(v,...a),bad=v<207||v>253,warn=v>248||v<212,
  k=bad?'red':warn?'amb':'g';
  return`<div class="ph" style="--pc:${PC[i]}"><div class="ch"><span class="nm"><i></i>L${i+1}</span><span class="pill" style="background:color-mix(in oklch,var(--${k}) 18%,transparent);color:${bad?'var(--red)':warn?'var(--wrt)':'var(--okt)'}">${bad?'poza normą':warn?'blisko granicy':'w normie'}</span></div>
<div class="vv"><b style="${bad?'color:var(--red)':''}">${fx(v,1)}</b><span class="sub">V</span></div>
<div class="vb"><div class="nb"></div><div class="rg" style="left:${vp(mn).toFixed(1)}%;right:${(100-vp(mx)).toFixed(1)}%"></div><div class="pt" style="left:${vp(v).toFixed(1)}%"></div></div>
<div class="vmm"><span>min <b>${fx(mn,1)}</b></span><span>maks. <b>${fx(mx,1)}</b></span></div></div>`}).join(''):'<div class="empty">Brak danych o napięciach</div>';
 const ok=VB.length>1;$('vempty').style.display=ok?'none':'';
 if(!ok){$('vl').innerHTML='';$('vticks').innerHTML='';$('vspan').textContent='brak danych';return}
 const x0=VB[0].x,x1=Date.now(),sp=Math.max(1,x1-x0),Xv=x=>((x-x0)/sp*600).toFixed(1),Yv=v=>clamp((260-v)/60*160,0,160).toFixed(1);
 $('vl').innerHTML=[0,1,2].map(i=>{const pts=VB.filter(b=>b.v[i]!=null);return pts.length>1?`<path d="M${pts.map(b=>Xv(b.x)+' '+Yv(b.v[i])).join(' L')}" fill="none" stroke="${PC[i]}" stroke-width="1.75" stroke-linejoin="round" vector-effect="non-scaling-stroke"/>`:''}).join('');
 $('vticks').innerHTML=[0,1,2,3,4].map(k=>`<span>${k==4?'teraz':hhmm(x0+sp*k/4)}</span>`).join('');$('vspan').textContent=dur(Math.round(sp/1000))}
// Odchylenia napiecia: VL = [{s,e,p,h,mn,mx,o}] (czas unix, faza, przepiecie, min/maks V, trwa)
let VL=[],vSync=0,VN=[207,253];
async function loadVolt(){try{const j=await(await fetch('/voltlog',{cache:'no-store'})).json();VN=[j.min,j.max];
 VL=j.e.map(r=>({s:r[0],e:r[1],p:r[2],h:r[3],mn:r[4]/10,mx:r[5]/10,o:r[6]==1}));vSync=Date.now();voltLog()}catch(e){}}
const dts=t=>new Date(t*1000).toLocaleString('pl-PL',{dateStyle:'short',timeStyle:'medium'}),vk=o=>o.h?'przepięcie':'zaniżenie',
 vu=v=>fx(v,1)+' V',isoD=d=>new Date(d-d.getTimezoneOffset()*60000).toISOString().slice(0,10),
 dm2=t=>{const d=new Date(t*1000);return String(d.getDate()).padStart(2,'0')+'.'+String(d.getMonth()+1).padStart(2,'0')+' '+hhmm(t*1000)};
function voltLog(){
 const now=Date.now()/1000,n=s=>VL.filter(o=>o.e>now-s).length;
 $('vsum').textContent=VL.length?`Zapisanych zdarzeń: ${VL.length} · ostatnia doba: ${n(86400)} · 7 dni: ${n(604800)}`:'Brak zapisanych odchyleń.';
 $('vtab').innerHTML=VL.slice().sort((a,b)=>b.s-a.s).slice(0,6).map(o=>{const c=o.h?'red':'b';
  return`<div class="dv"><span class="pill" style="background:color-mix(in oklch,var(--${c}) 16%,transparent);color:var(--${c})">${o.h?'za wysokie':'za niskie'}</span><span class="sub">${dm2(o.s)}</span><b>L${o.p} · ${vu(o.h?o.mx:o.mn)}</b><span class="sub r">${o.o?'trwa':dur(o.e-o.s)}</span></div>`}).join('')}
function vRange(){const f=$('vfrom').value,t=$('vto').value,a=f?new Date(f+'T00:00').getTime()/1000:0,b=t?new Date(t+'T00:00').getTime()/1000+86400:4e9;
 return{f,t,ev:VL.filter(o=>o.e>=a&&o.s<b).sort((x,y)=>x.s-y.s)}}
function vStats(ev){return[1,2,3].map(p=>{const e=ev.filter(o=>o.p==p);return{p,n:e.length,lo:e.filter(o=>!o.h).length,hi:e.filter(o=>o.h).length,
 t:e.reduce((a,o)=>a+o.e-o.s,0),mn:e.length?Math.min(...e.map(o=>o.mn)):null,mx:e.length?Math.max(...e.map(o=>o.mx)):null}})}
const meterName=()=>{const m=J&&J.meter||{};return m.serial||m.id||''};
function dl(name,text,type){const a=document.createElement('a');a.href=URL.createObjectURL(new Blob([text],{type}));
 a.download=name;a.click();setTimeout(()=>URL.revokeObjectURL(a.href),1000)}
function csv(name,rows){dl(name,'﻿'+rows.map(r=>r.join(';')).join('\r\n'),'text/csv')}
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
// ---- ustawienia ----
let pf=0;
function settings(j){
 if(!pf){pf=1;const pv=v=>v==null?'':String(v).replace('.',',');[1,2,3,4].forEach(t=>$('p'+t).value=pv(j.price&&j.price[t-1]));$('fee').value=j.fee_month>0?pv(j.fee_month):'';$('tar').value=j.tariff||0;tarLabels()}
 $('mid').placeholder=j.meter_filter||'np. 12345678';
 const m=j.meter||{};$('rec').textContent=m.records||'–';$('cpl').textContent=j.cloud&&j.cloud.last_payload?JSON.stringify(j.cloud.last_payload,null,1):'–';
 $('dev').innerHTML='<tr><th>ID</th><th>Prod.</th><th>Ramek</th><th>RSSI</th><th>Ost.</th><th>Status</th><th>Ostatnia ramka</th></tr>'+
  j.devices.sort((a,b)=>b.count-a.count).map(d=>`<tr><td>${esc(d.id)}</td><td>${esc(d.mfr)}</td><td>${d.count}</td><td>${d.rssi}</td><td>${dur(d.age_s)}</td><td>${esc(d.result)}</td><td class="hex">${esc(d.hex)}</td></tr>`).join('');
 const c=j.cloud;if(!c)return;const k=c.registered?(c.error?'amb':'g'):'muted';
 $('cpill').textContent=c.registered?'zarejestrowany':'niezarejestrowany';$('cpill').style.cssText=`background:color-mix(in oklch,var(--${k}) 16%,transparent);color:var(--${k=='g'?'okt':k=='amb'?'wrt':'muted'})`;
 $('cst').textContent=c.registered?'Device ID '+c.id+' · wysłano '+c.sent+' (co '+c.interval_s+' s)'+(c.last_age_s!=null?' · ostatnia wysyłka '+dur(c.last_age_s)+' temu, kod '+c.last_code+(c.error?' – '+c.error:''):''):'Odczyty nie są wysyłane.';
 $('capp').style.display=c.registered&&c.app_user?'':'none';$('cu').textContent=c.app_user||'';$('creg').style.display=c.registered?'none':'';$('cdel').style.display=c.registered?'':'none'}
let pwShown=0;
$('cpb').onclick=async()=>{if(pwShown){pwShown=0;$('cp').textContent='••••••';$('cpb').textContent='Pokaż';return}
 try{const r=await fetch('/cloudapp');if(r.ok){const a=await r.json();$('cp').textContent=a.pass||'–';pwShown=1;$('cpb').textContent='Ukryj'}}catch(e){}};
// ---- diagnostyka ----
function rfStats(j){
 $('rf').innerHTML=[['Moduł CC1101',j.radio_ok?'wykryty':'brak'],['Tryb','T1 · 868,95 MHz'],['Ramki poprawne',fx(j.frames-j.crc_errors)],['Ramki z błędem CRC',fx(j.crc_errors)],
  ['Synchronizacje (w tym szum)',fx(j.sync)],['Poziom szumu',j.noise_dbm!=null?j.noise_dbm+' dBm':'–']].map(x=>`<div><span>${x[0]}</span><b>${esc(x[1])}</b></div>`).join('');
 let t='';if(!j.radio_ok)t='';else if(j.sync_c1>=3&&j.sync_c1*1000>j.sync_bad)t='Wykryto transmisje w trybie C1 ('+j.sync_c1+') – ten firmware odbiera tylko tryb T1.';
 else if(!j.frames&&j.uptime_s>180)t='Przez '+dur(j.uptime_s)+' nie odebrano żadnej ramki wM-Bus. Jeśli test połączeń jest OK – licznik nie nadaje (interfejs HAN niewłączony przez TAURON) albo problem z anteną / modułem (np. wersja 433 MHz).';
 else if(j.frames>0&&j.crc_errors>j.frames/2)t='Większość ramek z błędem CRC – słaby sygnał: sprawdź antenę, zbliż moduł do licznika.';
 $('rfn').textContent=t}
const SC={ok:'var(--g)',warn:'var(--amb)',bad:'var(--red)'};
async function selftest(){const b=$('stb');stRun=1;b.disabled=true;b.textContent='Sprawdzam…';
 try{const r=await(await fetch('/selftest',{method:'POST'})).json();
  const sum={ok:'Wszystkie połączenia w porządku.',warn:'Są drobne uwagi – zobacz żółte pozycje.',bad:'Wykryto problem z połączeniem – przewody oznaczone na czerwono.'}[r.result];
  $('stt').innerHTML=`<div class="stres" style="--sc:${SC[r.result]}">${sum}${r.noise_dbm!=null?' Poziom szumu radia: '+r.noise_dbm+' dBm.':''}</div>`+
   '<div class="prow hd"><b>CC1101</b><span></span><b>NodeMCU</b><span></span></div>'+
   r.pins.map(p=>`<div class="prow ${p.s}" style="--sc:${SC[p.s]}"><b>${p.cc}</b><i></i><b>${p.esp}</b><small>${esc(p.m||'')}</small></div>`).join('')}
 catch(e){$('stt').innerHTML='<div class="stres" style="--sc:var(--red)">Nie udało się wykonać testu.</div>'}
 b.disabled=false;b.textContent='Sprawdź ponownie';load()}
$('stb').onclick=selftest;
// ---- start ----
addEventListener('resize',()=>{if(tab=='pulpit'){draw();drawMonths()}});
segOn('ivs','v',ivs);segOn('rg','r',range);segOn('mr','m',mRange);
{const h=location.hash.slice(1);go(h=='ustawienia'||h=='diag'?h:'pulpit');if(h=='napiecia'){history.replaceState(null,'','#napiecia');setTimeout(()=>$('napiecia').scrollIntoView(),300)}}
sched(0);
</script></body></html>)HTML";
