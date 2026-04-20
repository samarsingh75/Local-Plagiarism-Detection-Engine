const express = require('express');
const { exec } = require('child_process');
const fs = require('fs');
const path = require('path');

const app = express();
app.use(express.json());
app.use((req, res, next) => {
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type');
    next();
});

// Correct path (keep ONLY this one)
const exePath = path.join(__dirname, 'plagcheck.exe');

app.post('/compare', (req, res) => {
    const { text1, text2 } = req.body;

    fs.writeFileSync(path.join(__dirname, 'file1.txt'), text1);
    fs.writeFileSync(path.join(__dirname, 'file2.txt'), text2);

   exec('.\\plagcheck.exe file1.txt file2.txt', { cwd: __dirname }, (err, stdout, stderr) => {

        console.log("🔍 EXE PATH:", exePath);
        console.log("📤 STDOUT:", stdout);
        console.log("⚠️ STDERR:", stderr);
        console.log("❌ ERROR:", err);

        if (err) {
            return res.json({ error: stderr || err.message });
        }

        res.json({ result: stdout });
    });
});

app.listen(3000, () => {
    console.log("🚀 Server running at http://localhost:3000");
    console.log("📂 Using exe at:", exePath);
});