import fs from "node:fs/promises";
import { FileBlob, PresentationFile } from "@oai/artifact-tool";

const source = "F:\\Dev\\FortRogue\\docs\\CHARACTER_SETTING.pptx";
const output = "C:\\Users\\csh\\AppData\\Local\\Temp\\codex-presentations\\manual-20260701-character-setting\\character-setting\\tmp\\template-inspect-full.ndjson";

const presentation = await PresentationFile.importPptx(await FileBlob.load(source));
const snapshot = await presentation.inspect({
  kind: "slide,textbox,shape,image,table,chart",
  maxChars: 300000,
});
await fs.writeFile(output, snapshot.ndjson, "utf8");
console.log(output);
