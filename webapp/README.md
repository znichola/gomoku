# Frontend app

It's built with elm (functional SPA) & picocss (theme).

Make project and output html, or main.js.
```bash
elm make srce/Main.elm --output=main.js
```

Live reload and file hosting, requiers elm-live `npm i -g elm-live`.
```bash
elm-live src/Main.elm --verbose --open --hot -- --output=main.js
```
