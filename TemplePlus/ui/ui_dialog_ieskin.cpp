#include "stdafx.h"
#include <set>
#include "ui_dialog_ieskin.h"

#include <infrastructure/json11.hpp>
#include <infrastructure/stringutil.h>
#include <infrastructure/vfs.h>
#include <graphics/device.h>
#include <graphics/textengine.h>
#include <graphics/shaperenderer2d.h>

#include "tig/tig_startup.h"
#include "ui.h"
#include "critter.h"
#include "description.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/legacysystems.h"
#include "tig/tig_texture.h"
#include "ui_render.h"
#include "ui_assets.h"
#include <temple/dll.h>

IeDialogSkins ieDialogSkins;

static const char *kSelectionFile = "art\\interface\\dialog_ie\\skins.json";

struct SkinFont {
	std::string face;
	float point = 12;
	bool upper = false;
};

struct IeDialogSkins::Skin {
	std::string name, layout = "panel";
	int pw = 0, ph = 0, bottom = 0;
	gfx::TextureRef panel, history;
	TigRect portrait, npcText, replies, historyRect, historyText, textRect, contRect, histBtnRect, scrollRect;
	bool hasPortrait = false, hasHistory = false, hasCont = false, hasHistBtn = false, hasScroll = false;
	std::vector<gfx::TextureRef> contFrames, histFrames;
	gfx::TextureRef scrollUp, scrollDown, scrollKnob, scrollTrough;
	std::string contText = "Continue", endText = "End Dialogue";
	std::vector<std::string> contMarkers{ "[Continue]" }, endMarkers{ "[End dialogue]" };
	SkinFont font, contFont;
	uint32_t contColor = 0xFFCECECE;
	uint32_t cNpcText = 0xFFCECECE, cNpcName = 0xFFFFE682, cReply = 0xFFFF2E21, cReplyHover = 0xFFFFFFFF,
		cReplyDown = 0xFFFFFFFF, cHistPc = 0xFFA0A0A0, cHistNpc = 0xFFC8C8C8;
	int replyGap = 2;
	int designW = 1024, designH = 768; // layout coordinates are for this screen size (IE GUI resolution)
	float minPoint = 11;

	// copy scaled to the actual screen (only shrinks: Temple+ commonly runs at 1024x640 or wider)
	Skin Scaled(int screenW, int screenH) const {
		Skin s = *this;
		float k = std::min(1.0f, std::min((float)screenW / designW, (float)screenH / designH));
		s.scale = k;
		if (k >= 0.999f) return s;
		auto R = [k](TigRect &r) {
			r = TigRect((int)lroundf(r.x * k), (int)lroundf(r.y * k), (int)lroundf(r.width * k), (int)lroundf(r.height * k));
		};
		for (auto r : { &s.portrait, &s.npcText, &s.replies, &s.historyRect, &s.historyText, &s.textRect, &s.contRect,
			&s.histBtnRect, &s.scrollRect })
			R(*r);
		s.pw = (int)lroundf(pw * k); s.ph = (int)lroundf(ph * k); s.bottom = (int)lroundf(bottom * k);
		s.replyGap = std::max(1, (int)lroundf(replyGap * k));
		s.font.point = std::max(font.point * k, std::min(font.point, minPoint));
		s.contFont.point = std::max(contFont.point * k, std::min(contFont.point, minPoint));
		return s;
	}
	float scale = 1;
};

// ---------------------------------------------------------------------------------------------------- helpers
static gfx::TextEngine &TE() {
	return tig->GetRenderingDevice().GetTextEngine();
}

static uint32_t ParseColor(const json11::Json &j, uint32_t def) {
	if (!j.is_string()) return def;
	try {
		return (uint32_t)std::stoul(j.string_value(), nullptr, 16);
	}
	catch (...) {
		return def;
	}
}

static TigRect ParseRect(const json11::Json &j) {
	auto &a = j.array_items();
	if (a.size() < 4) return TigRect(0, 0, 0, 0);
	return TigRect(a[0].int_value(), a[1].int_value(), a[2].int_value(), a[3].int_value());
}

static gfx::TextureRef Tex(const json11::Json &j) {
	if (!j.is_string() || j.string_value().empty()) return gfx::TextureRef();
	auto t = tig->GetRenderingDevice().GetTextures().Resolve(j.string_value(), false);
	if (!t || !t->IsValid()) {
		logger->warn("IE dialog skin: missing texture {}", j.string_value());
		return gfx::TextureRef();
	}
	return t;
}

static SkinFont ParseFont(const json11::Json &j, const SkinFont &def) {
	SkinFont f = def;
	if (!j.is_object()) return f;
	if (j["face"].is_string()) f.face = j["face"].string_value();
	if (j["point"].is_number()) f.point = (float)j["point"].number_value();
	if (j["upper"].is_bool()) f.upper = j["upper"].bool_value();
	static std::set<std::string> sAdded; // each font file is added once (AddFont reloads the whole collection)
	if (j["file"].is_string() && !sAdded.count(j["file"].string_value()) && !TE().HasFontFamily(f.face)
		&& TE().FindFontFamily(f.face).empty() && vfs->FileExists(j["file"].string_value())) {
		sAdded.insert(j["file"].string_value());
		// fonts\*.ttf is only scanned at startup, before a module's own folder is mounted: add the skin's font now
		logger->info("IE dialog skin: adding font {}", j["file"].string_value());
		TE().AddFont(j["file"].string_value());
	}
	// DirectWrite groups legacy fonts by their WWS family, dropping style words from the name table family
	// (Wine lists "R PostAntiqua Roman" as "R PostAntiqua Roman  05554"): match by prefix, then shorter names
	if (!TE().HasFontFamily(f.face)) {
		auto found = TE().FindFontFamily(f.face);
		for (auto cand = f.face; found.empty() && cand.find(' ') != std::string::npos; ) {
			cand = cand.substr(0, cand.rfind(' '));
			found = TE().FindFontFamily(cand);
		}
		if (!found.empty()) {
			logger->info("IE dialog skin: font '{}' is registered as '{}'", f.face, found);
			f.face = found;
		}
	}
	if (!TE().HasFontFamily(f.face)) {
		auto fb = j["fallback"].is_string() ? j["fallback"].string_value() : std::string("Junicode");
		logger->info("IE dialog skin: font '{}' not installed, using '{}'", f.face, fb);
		f.face = fb;
	}
	return f;
}

static gfx::TextStyle Style(const SkinFont &f, uint32_t argb) {
	gfx::TextStyle st;
	st.fontFace = f.face.c_str();
	st.pointSize = f.point;
	st.foreground.primaryColor = XMCOLOR(argb);
	st.dropShadow = true;
	st.dropShadowBrush.primaryColor = XMCOLOR(0xC0000000);
	return st;
}

static gfx::FormattedText Fmt(const std::string &text, const gfx::TextStyle &st) {
	gfx::FormattedText ft;
	ft.text = local_to_ucs2(text);
	ft.defaultStyle = st;
	return ft;
}

static int MeasureH(const gfx::FormattedText &ft, int width) {
	if (ft.text.empty()) return 0;
	gfx::TextMetrics m;
	m.width = width;
	m.height = 4000;
	TE().MeasureText(ft, m);
	return (int)m.height;
}

static void DrawTex(gfx::Texture &t, float x, float y, float w, float h,
	float u0 = 0, float v0 = 0, float u1 = 1, float v1 = 1) {
	std::array<gfx::Vertex2d, 4> v;
	const float xs[4] = { x, x + w, x + w, x }, ys[4] = { y, y, y + h, y + h };
	const float us[4] = { u0, u1, u1, u0 }, vs[4] = { v0, v0, v1, v1 };
	for (int i = 0; i < 4; i++) {
		v[i].pos = XMFLOAT4(xs[i], ys[i], 0.5f, 1);
		v[i].normal = XMFLOAT4(0, 0, -1, 0);
		v[i].diffuse = XMCOLOR(0xFFFFFFFF);
		v[i].uv = XMFLOAT2(us[i], vs[i]);
	}
	tig->GetShapeRenderer2d().DrawRectangle(v, &t);
}

// Draws a framed panel texture into any rect: corners kept, edges and centre stretched.
static void DrawFramed(gfx::Texture &t, const TigRect &r, float k = 1, int L = 20, int T = 12, int R = 20, int B = 26) {
	auto sz = t.GetSize();
	float tw = (float)sz.width, th = (float)sz.height;
	if (r.width == sz.width && r.height == sz.height) {
		DrawTex(t, (float)r.x, (float)r.y, (float)r.width, (float)r.height);
		return;
	}
	float sx[4] = { 0, (float)L, tw - R, tw }, sy[4] = { 0, (float)T, th - B, th };
	float dx[4] = { (float)r.x, r.x + L * k, r.x + r.width - R * k, (float)(r.x + r.width) };
	float dy[4] = { (float)r.y, r.y + T * k, r.y + r.height - B * k, (float)(r.y + r.height) };
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++) {
			if (dx[i + 1] <= dx[i] || dy[j + 1] <= dy[j]) continue;
			DrawTex(t, dx[i], dy[j], dx[i + 1] - dx[i], dy[j + 1] - dy[j],
				sx[i] / tw, sy[j] / th, sx[i + 1] / tw, sy[j + 1] / th);
		}
}

static bool Matches(const std::string &text, const std::vector<std::string> &markers) {
	std::string t = text;
	trim(t);
	for (auto &m : markers)
		if (!_stricmp(t.c_str(), m.c_str())) return true;
	return false;
}

static std::string Upper(std::string s) {
	for (auto &c : s) c = (char)toupper((unsigned char)c);
	return s;
}

static std::string ReplyText(int idx, const std::string &text) {
	return fmt::format("{} - {}", idx + 1, text);
}

// ---------------------------------------------------------------------------------------------------- selection
IeDialogSkins::IeDialogSkins() = default;
IeDialogSkins::~IeDialogSkins() = default;

void IeDialogSkins::Deactivate() {
	mCur = -1; mCurDialog = -1; mScaled.reset(); mPortraitDialog = -1;
}

void IeDialogSkins::LoadSelection() {
	mSelectionLoaded = true;
	if (!vfs->FileExists(kSelectionFile)) return;
	std::string err;
	auto j = json11::Json::parse(vfs->ReadAsString(kSelectionFile), err);
	if (!err.empty()) {
		logger->error("IE dialog skin: cannot parse {}: {}", kSelectionFile, err);
		return;
	}
	mDefault = j["default"].string_value();
	for (auto &kv : j["dialogs"].object_items())
		mByDialog.emplace_back(atoi(kv.first.c_str()), kv.second.string_value());
	for (auto &kv : j["portraits"].object_items())
		mPortraits.emplace_back(atoi(kv.first.c_str()), kv.second.string_value());
	logger->info("IE dialog skin: selection loaded (default '{}', {} dialogs)", mDefault, mByDialog.size());
}

int IeDialogSkins::LoadSkin(const std::string &name) {
	for (size_t i = 0; i < mSkins.size(); i++)
		if (mSkins[i]->name == name) return (int)i;
	auto path = fmt::format("art\\interface\\dialog_ie\\{}\\skin.json", name);
	if (!vfs->FileExists(path)) {
		logger->error("IE dialog skin: {} not found", path);
		return -1;
	}
	std::string err;
	auto j = json11::Json::parse(vfs->ReadAsString(path), err);
	if (!err.empty()) {
		logger->error("IE dialog skin: cannot parse {}: {}", path, err);
		return -1;
	}
	auto s = std::make_unique<Skin>();
	s->name = name;
	if (j["layout"].is_string()) s->layout = j["layout"].string_value();
	if (j["design"].is_array() && j["design"].array_items().size() == 2) {
		s->designW = j["design"][0].int_value(); s->designH = j["design"][1].int_value();
	}
	if (j["font"]["min_point"].is_number()) s->minPoint = (float)j["font"]["min_point"].number_value();
	auto &p = j["panel"];
	s->panel = Tex(p["image"]);
	s->pw = p["w"].int_value(); s->ph = p["h"].int_value(); s->bottom = p["bottom"].int_value();
	if (s->panel && (!s->pw || !s->ph)) {
		auto sz = s->panel->GetSize(); s->pw = sz.width; s->ph = sz.height;
	}
	s->hasPortrait = j["portrait"].is_object();
	s->portrait = ParseRect(j["portrait"]["rect"]);
	s->npcText = ParseRect(j["npc_text"]["rect"]);
	s->replies = ParseRect(j["replies"]["rect"]);
	if (j["replies"]["gap"].is_number()) s->replyGap = j["replies"]["gap"].int_value();
	s->textRect = ParseRect(j["text"]["rect"]);
	if (j["history"].is_object()) {
		s->hasHistory = true;
		s->history = Tex(j["history"]["image"]);
		s->historyRect = ParseRect(j["history"]["rect"]);
		s->historyText = ParseRect(j["history"]["text_rect"]);
	}
	SkinFont defFont{ "Junicode", 12, false };
	s->font = ParseFont(j["font"], defFont);
	if (j["continue"].is_object()) {
		auto &c = j["continue"];
		s->hasCont = true;
		s->contRect = ParseRect(c["rect"]);
		for (auto &f : c["frames"].array_items()) s->contFrames.push_back(Tex(f));
		if (c["text_continue"].is_string()) s->contText = c["text_continue"].string_value();
		if (c["text_end"].is_string()) s->endText = c["text_end"].string_value();
		s->contFont = ParseFont(c["font"], s->font);
		s->contColor = ParseColor(c["color"], s->contColor);
	}
	if (j["markers"].is_object()) {
		s->contMarkers.clear(); s->endMarkers.clear();
		for (auto &m : j["markers"]["continue"].array_items()) s->contMarkers.push_back(m.string_value());
		for (auto &m : j["markers"]["end"].array_items()) s->endMarkers.push_back(m.string_value());
	}
	if (j["history_button"].is_object()) {
		s->hasHistBtn = true;
		s->histBtnRect = ParseRect(j["history_button"]["rect"]);
		for (auto &f : j["history_button"]["frames"].array_items()) s->histFrames.push_back(Tex(f));
	}
	if (j["scrollbar"].is_object()) {
		auto &a = j["scrollbar"]["art"];
		s->hasScroll = j["scrollbar"]["rect"].is_array();
		s->scrollRect = ParseRect(j["scrollbar"]["rect"]);
		s->scrollUp = Tex(a["up"]); s->scrollDown = Tex(a["down"]);
		s->scrollKnob = Tex(a["knob"]); s->scrollTrough = Tex(a["trough"]);
	}
	auto &c = j["colors"];
	s->cNpcText = ParseColor(c["npc_text"], s->cNpcText);
	s->cNpcName = ParseColor(c["npc_name"], s->cNpcName);
	s->cReply = ParseColor(c["reply"], s->cReply);
	s->cReplyHover = ParseColor(c["reply_hover"], s->cReplyHover);
	s->cReplyDown = ParseColor(c["reply_down"], s->cReplyDown);
	s->cHistPc = ParseColor(c["history_pc"], s->cHistPc);
	s->cHistNpc = ParseColor(c["history_npc"], s->cHistNpc);
	logger->info("IE dialog skin: loaded '{}' (layout {}, panel {}x{}, font '{}' {}pt)", name, s->layout, s->pw, s->ph,
		s->font.face, s->font.point);
	mSkins.push_back(std::move(s));
	return (int)mSkins.size() - 1;
}

bool IeDialogSkins::Select(int dialogId) {
	if (!mSelectionLoaded) LoadSelection();
	if (dialogId == mCurDialog) return mCur >= 0;
	mCurDialog = dialogId;
	mHistoryScroll = 0;
	mScaled.reset();
	std::string name = mDefault;
	for (auto &kv : mByDialog)
		if (kv.first == dialogId) name = kv.second;
	mCur = name.empty() ? -1 : LoadSkin(name);
	if (!name.empty())
		logger->info("IE dialog skin: dialog {} -> '{}'{}", dialogId, name, mCur < 0 ? " (failed)" : "");
	return mCur >= 0;
}

const std::string &IeDialogSkins::CurrentName() const {
	static std::string none;
	return mCur >= 0 ? mSkins[mCur]->name : none;
}

std::string IeDialogSkins::PortraitFor(const IeDlgView &view) {
	for (auto &kv : mPortraits)
		if (kv.first == view.dialogId) return kv.second;
	return std::string();
}

// The NPC's own ToEE portrait through the engine's portrait asset loader (handles portrait packs); the biggest variant.
static int ToeePortrait(objHndl npc, int &w, int &h) {
	if (!npc) return 0;
	auto id = critterSys.GetPortraitId(npc);
	int best = 0;
	w = h = 0;
	for (int sub = 0; sub < 3; sub++) { // the largest portrait size wins
		int tex = 0;
		if (temple::GetRef<int(__cdecl)(UiAssetType, int, int*, int)>(0x1004A360)(UiAssetType::Portraits, id, &tex, sub) || !tex)
			continue;
		TigTextureRegistryEntry e;
		if (!textureFuncs.LoadTexture(tex, &e) && e.width * e.height > w * h) {
			best = tex; w = e.width; h = e.height;
		}
	}
	logger->info("IE dialog skin: ToEE portrait id {} -> texture {} {}x{}", id, best, w, h);
	return best;
}

// ---------------------------------------------------------------------------------------------------- layout
IeDlgLayout IeDialogSkins::Layout(const IeDlgView &v) {
	IeDlgLayout L;
	if (mCur < 0) return L;
	auto key = (mCur * 8192 + v.screenW) * 8192 + v.screenH;
	if (!mScaled || key != mScaledKey) {
		mScaled = std::make_unique<Skin>(mSkins[mCur]->Scaled(v.screenW, v.screenH));
		mScaledKey = key;
		logger->info("IE dialog skin: '{}' scaled x{:.3f} for {}x{} (panel {}x{}, {}pt)", mScaled->name, mScaled->scale,
			v.screenW, v.screenH, mScaled->pw, mScaled->ph, mScaled->font.point);
	}
	auto &s = *mScaled;
	L.historyMode = v.historyMode;
	auto px = (v.screenW - s.pw) / 2;
	auto txt = Style(s.font, s.cReply);
	int replyH[5] = { 0, };
	for (int i = 0; i < v.replyCount && i < 5; i++)
		replyH[i] = MeasureH(Fmt(ReplyText(i, v.replies[i]), txt), s.layout == "log" ? s.textRect.width : s.replies.width);
	if (s.hasCont && v.replyCount == 1) {
		L.continueIsEnd = Matches(v.replies[0], s.endMarkers);
		L.continueMode = L.continueIsEnd || Matches(v.replies[0], s.contMarkers);
	}

	if (s.layout == "log") {
		int py = v.screenH - s.bottom - s.ph;
		L.panelX = px; L.panelY = py; L.panelW = s.pw; L.panelH = s.ph;
		TigRect t(px + s.textRect.x, py + s.textRect.y, s.textRect.width, s.textRect.height);
		int y = t.y + t.height;
		for (int i = std::min(v.replyCount, 5) - 1; i >= 0; i--) {
			y -= replyH[i] + s.replyGap;
			L.replies[i] = TigRect(t.x, y, t.width, replyH[i]);
		}
		L.wnd = TigRect(px, py, s.pw, s.ph);
		// the reply window sits above the main window: keep it to the replies so the history button gets its clicks
		L.responseWnd = v.replyCount > 0 ? TigRect(t.x, L.replies[0].y, t.width, t.y + t.height - L.replies[0].y) : TigRect(0, 0, 0, 0);
		L.headBtn = s.hasHistBtn ? TigRect(px + s.histBtnRect.x, py + s.histBtnRect.y, s.histBtnRect.width, s.histBtnRect.height)
			: TigRect(0, 0, 0, 0);
		return L;
	}

	// "panel" (EE): panel grows upwards when the NPC text plus replies don't fit, like the EE resizeDialog()
	auto npcFt = Fmt(v.npcText, Style(s.font, s.cNpcText));
	int npcH = std::max(MeasureH(npcFt, s.npcText.width), (int)s.npcText.height);
	int listTop = std::max((int)s.replies.y, s.npcText.y + npcH + (s.replies.y - s.npcText.y - s.npcText.height));
	int listH = 0;
	if (!L.continueMode)
		for (int i = 0; i < v.replyCount && i < 5; i++) listH += replyH[i] + s.replyGap;
	int bottomPad = s.ph - (s.replies.y + s.replies.height);
	int ph = std::max(s.ph, listTop + listH + bottomPad);
	int py = v.screenH - s.bottom - ph;
	L.panelX = px; L.panelY = py; L.panelW = s.pw; L.panelH = ph;
	int y = py + listTop;
	for (int i = 0; i < v.replyCount && i < 5; i++) {
		if (L.continueMode) break;
		L.replies[i] = TigRect(px + s.replies.x, y, s.replies.width, replyH[i]);
		y += replyH[i] + s.replyGap;
	}
	TigRect panelRect(px, py, s.pw, ph);
	L.wnd = panelRect;
	if (v.historyMode && s.hasHistory) {
		L.wnd.y = py + s.historyRect.y;
		L.wnd.height = ph - s.historyRect.y;
	}
	// the reply window sits above the main window: keep it to the reply list so the history button gets its clicks
	L.responseWnd = TigRect(px, py + listTop, s.pw, ph - listTop);
	if (L.continueMode) {
		TigRect c(v.screenW / 2 + s.contRect.x, v.screenH + s.contRect.y, s.contRect.width, s.contRect.height);
		L.replies[0] = c;
		auto bottom = std::max(L.responseWnd.y + L.responseWnd.height, c.y + c.height);
		auto right = std::max(L.responseWnd.x + L.responseWnd.width, c.x + c.width);
		L.responseWnd.x = std::min(L.responseWnd.x, c.x);
		L.responseWnd.width = right - L.responseWnd.x;
		L.responseWnd.height = bottom - L.responseWnd.y;
	}
	if (!v.active) { // history view after the conversation: only the log
		L.responseWnd = TigRect(0, 0, 0, 0);
	}
	L.headBtn = s.hasHistBtn ? TigRect(px + s.histBtnRect.x, py + s.histBtnRect.y, s.histBtnRect.width, s.histBtnRect.height)
		: TigRect(0, 0, 0, 0);
	return L;
}

// ---------------------------------------------------------------------------------------------------- rendering
static void DrawHistory(const IeDialogSkins::Skin &s, const IeDlgView &v, const TigRect &area, int bottomY, int skip) {
	// newest at the bottom, older lines above; clipped to the area
	TE().SetScissorRect(area);
	int y = bottomY;
	int n = 0;
	for (auto &h : v.history) {
		if (n++ < skip) continue;
		// ToEE logs lines as "(Speaker)  text"; the IE log shows "Speaker: text" with the name highlighted
		std::string line = h.second, name;
		if (line.size() > 2 && line[0] == '(') {
			auto close = line.find(')');
			if (close != std::string::npos && close < 64) {
				name = line.substr(1, close - 1) + ": ";
				line = line.substr(close + 1);
				ltrim(line);
			}
		}
		auto ft = Fmt(name + line, Style(s.font, h.first ? s.cHistPc : s.cHistNpc));
		if (!name.empty()) {
			gfx::ConstrainedTextStyle cs(Style(s.font, s.cNpcName));
			cs.length = (uint32_t)local_to_ucs2(name).size();
			ft.formats.push_back(cs);
		}
		int hh = MeasureH(ft, area.width);
		y -= hh + 2;
		if (y < area.y) break; // whole lines only, like the IE log
		TE().RenderText(TigRect(area.x, y, area.width, hh + 4), ft);
	}
	TE().ResetScissorRect();
}

static void DrawNpcLine(const IeDialogSkins::Skin &s, const IeDlgView &v, const TigRect &r, bool withName) {
	std::string name;
	if (withName && v.npc) {
		auto n = description.getDisplayName(v.npc, v.pc);
		if (n && *n) name = std::string(n) + ": ";
	}
	auto ft = Fmt(name + v.npcText, Style(s.font, s.cNpcText));
	if (!name.empty()) {
		gfx::ConstrainedTextStyle cs(Style(s.font, s.cNpcName));
		cs.startChar = 0;
		cs.length = (uint32_t)local_to_ucs2(name).size();
		ft.formats.push_back(cs);
	}
	TE().RenderText(r, ft);
}

void IeDialogSkins::RenderMain(const IeDlgView &v, const IeDlgLayout &L) {
	if (mCur < 0) return;
	if (!mScaled) return;
	auto &s = *mScaled;
	TigRect panel(L.panelX, L.panelY, L.panelW, L.panelH);

	if (s.layout == "log") {
		if (s.panel) DrawFramed(*s.panel, panel, s.scale);
		TigRect t(panel.x + s.textRect.x, panel.y + s.textRect.y, s.textRect.width, s.textRect.height);
		int top = t.y + t.height;
		for (int i = 0; i < v.replyCount && i < 5; i++)
			if (L.replies[i].height) top = std::min(top, (int)L.replies[i].y);
		// current NPC line right above the replies, the log above it
		std::string name;
		if (v.npc) {
			auto n = description.getDisplayName(v.npc, v.pc);
			if (n && *n) name = std::string(n) + ": ";
		}
		int npcH = v.active ? MeasureH(Fmt(name + v.npcText, Style(s.font, s.cNpcText)), t.width) : 0;
		int npcY = top - 4 - npcH;
		TE().SetScissorRect(t);
		if (v.active) DrawNpcLine(s, v, TigRect(t.x, npcY, t.width, npcH + 4), true);
		TE().ResetScissorRect();
		DrawHistory(s, v, t, v.active ? npcY - 4 : t.y + t.height, (v.active ? 1 : 0) + mHistoryScroll);
		if (s.hasScroll && s.scrollUp && s.scrollDown) {
			// IE scrollbar: up arrow, tiled trough, knob, down arrow (frames of the CHU scrollbar BAM), scaled
			auto k = s.scale;
			auto su = s.scrollUp->GetSize(), sd = s.scrollDown->GetSize();
			float uw = su.width * k, uh = su.height * k, dw = sd.width * k, dh = sd.height * k;
			float cx = panel.x + s.scrollRect.x + s.scrollRect.width / 2.0f;
			float top = (float)(panel.y + s.scrollRect.y), bot = top + s.scrollRect.height;
			if (s.scrollTrough) {
				auto tsz = s.scrollTrough->GetSize();
				float tw = tsz.width * k, th = std::max(1.0f, tsz.height * k);
				for (float y = top + uh; y < bot - dh; y += th) {
					float hh = std::min(th, bot - dh - y);
					DrawTex(*s.scrollTrough, cx - tw / 2, y, tw, hh, 0, 0, 1, hh / th);
				}
			}
			DrawTex(*s.scrollUp, cx - uw / 2, top, uw, uh);
			DrawTex(*s.scrollDown, cx - dw / 2, bot - dh, dw, dh);
			if (s.scrollKnob) {
				auto ks = s.scrollKnob->GetSize();
				float kw = ks.width * k, kh = ks.height * k;
				// knob at the bottom = newest text; moves up while scrolling back through the log
				float range = std::max(0.0f, bot - dh - kh - (top + uh));
				float pos = std::min(1.0f, mHistoryScroll / std::max(1.0f, (float)v.history.size()));
				DrawTex(*s.scrollKnob, cx - kw / 2, bot - dh - kh - range * pos, kw, kh);
			}
		}
		return;
	}

	// "panel"
	if (v.historyMode && s.hasHistory) {
		TigRect hr(panel.x + s.historyRect.x, panel.y + s.historyRect.y, s.historyRect.width, s.historyRect.height);
		if (s.history) DrawFramed(*s.history, hr, s.scale);
		TigRect ht(hr.x + s.historyText.x, hr.y + s.historyText.y, s.historyText.width, s.historyText.height);
		DrawHistory(s, v, ht, ht.y + ht.height, (v.active ? 1 : 0) + mHistoryScroll);
	}
	if (!v.active) return;
	if (s.panel) DrawFramed(*s.panel, panel, s.scale);
	if (s.hasPortrait) {
		TigRect pr(panel.x + s.portrait.x, panel.y + s.portrait.y, s.portrait.width, s.portrait.height);
		tig->GetShapeRenderer2d().DrawRectangle((float)pr.x - 1, (float)pr.y - 1, (float)pr.width + 2, (float)pr.height + 2, 0xFF000000);
		if (mPortraitDialog != v.dialogId) {
			mPortraitDialog = v.dialogId;
			mPortraitTex = 0;
			auto path = PortraitFor(v);
			int texId = 0;
			TigTextureRegistryEntry e;
			if (!path.empty() && !textureFuncs.RegisterTexture(path.c_str(), &texId) && !textureFuncs.LoadTexture(texId, &e)) {
				mPortraitTex = texId; mPortraitW = e.width; mPortraitH = e.height;
			}
			if (!mPortraitTex)
				mPortraitTex = ToeePortrait(v.npc, mPortraitW, mPortraitH);
			logger->info("IE dialog skin: portrait for dialog {}: '{}' {}x{}{}", v.dialogId, path, mPortraitW, mPortraitH,
				mPortraitTex ? "" : " (not loaded)");
		}
		if (mPortraitTex && mPortraitW > 0 && mPortraitH > 0) {
			// fill the IE portrait slot (54x84 in the EE) and crop the ToEE portrait to its aspect
			float ta = (float)mPortraitW / mPortraitH, ra = (float)pr.width / pr.height;
			TigRect src(0, 0, mPortraitW, mPortraitH);
			if (ta > ra) { int w = (int)(mPortraitH * ra); src = TigRect((mPortraitW - w) / 2, 0, w, mPortraitH); }
			else { int h = (int)(mPortraitW / ra); src = TigRect(0, (mPortraitH - h) / 2, mPortraitW, h); }
			UiRenderer::DrawTexture(mPortraitTex, pr, src);
		}
	}
	auto npcFt = Fmt(v.npcText, Style(s.font, s.cNpcText));
	int npcH = std::max(MeasureH(npcFt, s.npcText.width), (int)s.npcText.height);
	DrawNpcLine(s, v, TigRect(panel.x + s.npcText.x, panel.y + s.npcText.y, s.npcText.width, npcH + 4), false);
}

void IeDialogSkins::RenderReply(const IeDlgView &v, const IeDlgLayout &L, int idx, int btnState) {
	if (mCur < 0 || idx < 0 || idx >= v.replyCount || idx >= 5 || !L.replies[idx].height) return;
	if (!mScaled) return;
	auto &s = *mScaled;
	auto st = (LgcyButtonState)btnState;
	auto &r = L.replies[idx];
	if (L.continueMode && idx == 0) {
		int frame = st == LgcyButtonState::Down ? 1 : (st == LgcyButtonState::Hovered ? 2 : 0);
		if (frame < (int)s.contFrames.size() && s.contFrames[frame])
			DrawTex(*s.contFrames[frame], (float)r.x, (float)r.y, (float)r.width, (float)r.height);
		auto label = L.continueIsEnd ? s.endText : s.contText;
		if (s.contFont.upper) label = Upper(label);
		auto style = Style(s.contFont, s.contColor);
		style.align = gfx::TextAlign::Center;
		auto ft = Fmt(label, style);
		int h = MeasureH(ft, r.width);
		TE().RenderText(TigRect(r.x, r.y + (r.height - h) / 2, r.width, h + 2), ft);
		return;
	}
	uint32_t color = s.cReply;
	if (st == LgcyButtonState::Hovered) color = s.cReplyHover;
	else if (st == LgcyButtonState::Down) color = s.cReplyDown;
	auto text = v.replies[idx];
	if (s.layout == "log" && Matches(text, s.endMarkers)) text = s.endText;
	else if (s.layout == "log" && Matches(text, s.contMarkers)) text = s.contText;
	TE().RenderText(TigRect(r.x, r.y, r.width, r.height + 4), Fmt(ReplyText(idx, text), Style(s.font, color)));
}

void IeDialogSkins::RenderHistoryButton(const IeDlgLayout &L, int btnState) {
	if (mCur < 0) return;
	if (!mScaled) return;
	auto &s = *mScaled;
	if (!s.hasHistBtn || s.histFrames.empty()) return;
	int frame = L.historyMode ? 1 : 0;
	if (s.layout == "log") frame = ((LgcyButtonState)btnState == LgcyButtonState::Down) ? 1 : 0;
	if (frame >= (int)s.histFrames.size() || !s.histFrames[frame]) frame = 0;
	if (!s.histFrames[frame]) return;
	auto sz = s.histFrames[frame]->GetSize();
	auto &r = L.headBtn;
	float w = sz.width * s.scale, h = sz.height * s.scale;
	DrawTex(*s.histFrames[frame], r.x + (r.width - w) / 2, r.y + (r.height - h) / 2, w, h);
}
