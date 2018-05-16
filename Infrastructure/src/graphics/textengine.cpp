
#include "graphics/device.h"
#include "graphics/textengine.h"
#include "platform/d3d.h"
#include "infrastructure/vfs.h"
#include "infrastructure/stringutil.h"
#include "infrastructure/logging.h"

#include <EASTL/vector.h>
#include <EASTL/hash_map.h>

#include <atlbase.h>
#include <atlcom.h>

#include <d2d1_1.h>
#include <d2d1_1helper.h>
#include <dwrite.h>

namespace gfx {
	
	class MemoryFontStream : public IDWriteFontFileStream {
	public:
		MemoryFontStream(const std::vector<uint8_t> &data) : mData(data) {}

		HRESULT __stdcall ReadFileFragment(
			void const** fragmentStart,
			uint64_t fileOffset,
			uint64_t fragmentSize,
			void** fragmentContext
			) override;

		void __stdcall ReleaseFileFragment(void* fragmentContext) override;

		HRESULT __stdcall GetFileSize(uint64_t* fileSize) override;

		HRESULT __stdcall GetLastWriteTime(uint64_t* lastWriteTime) override;

		// IUnknown implementation
		HRESULT __stdcall QueryInterface(REFIID uuid, LPVOID *objOut) override;
		ULONG __stdcall AddRef() override;
		ULONG __stdcall Release() override;

	private:
		const std::vector<uint8_t> &mData;
		volatile uint32_t mRefCount = 1;
	};

	/*
		Stores info about a certain text style.
	*/
	struct TextStyleState {
		CComPtr<IDWriteTextFormat> textFormat; // The backing DirectWrite text format
	};

	struct FontFile {
		std::string filename;
		std::vector<uint8_t> data;
	};

	// Hash that only uses the fields relevant to the text format
	struct TextFormatHash {
		size_t operator()(const gfx::TextStyle &style) const {
			// Do a composite hash
			auto hash = eastl::hash<eastl::string>()(style.fontFace)
				^ eastl::hash<bool>()(style.bold)
				^ eastl::hash<bool>()(style.trim)
				^ eastl::hash<bool>()(style.italic)
				^ eastl::hash<float>()(style.pointSize)
				^ eastl::hash<float>()(style.tabStopWidth)
				^ eastl::hash<uint32_t>()((uint32_t)style.align)
				^ eastl::hash<uint32_t>()((uint32_t)style.paragraphAlign)
				^ eastl::hash<bool>()(style.uniformLineHeight);

			if (style.uniformLineHeight) {
				hash ^= eastl::hash<float>()(style.lineHeight)
					^ eastl::hash<float>()(style.baseLine);
			}
			return hash;
		}
	};
	struct TextFormatEquals {
		bool operator()(const gfx::TextStyle &a, const gfx::TextStyle &b) const {
			if (!(a.fontFace == b.fontFace
				&& a.pointSize == b.pointSize
				&& a.bold == b.bold
				&& a.trim == b.trim
				&& a.italic == b.italic
				&& a.tabStopWidth == b.tabStopWidth
				&& a.align == b.align
				&& a.paragraphAlign == b.paragraphAlign
				&& a.uniformLineHeight == b.uniformLineHeight)) {
				return false;
			}
			if (a.uniformLineHeight) {
				return a.lineHeight == b.lineHeight
					|| a.baseLine == b.baseLine;
			}
			return true;
		}
	};

	// A hash map only using fields relevant to the text format
	using TextFormatsCache = eastl::hash_map<TextStyle, CComPtr<IDWriteTextFormat>, TextFormatHash, TextFormatEquals>;
	
	struct BrushHash {
		size_t operator()(const gfx::Brush &brush) const {
			auto result = eastl::hash<bool>()(brush.gradient)
				^ eastl::hash<uint32_t>()(brush.primaryColor.c);
			if (brush.gradient) {
				result ^= eastl::hash<uint32_t>()(brush.secondaryColor.c);
			}
			return result;
		}
	};
	struct BrushEquals {
		bool operator()(const gfx::Brush &a, const gfx::Brush &b) const {
			if (a.gradient != b.gradient
				|| a.primaryColor != b.primaryColor) {
				return false;
			}
			return !a.gradient ||
				a.secondaryColor == b.secondaryColor;
		}
	};

	using BrushCache = eastl::hash_map<Brush, CComPtr<ID2D1Brush>, BrushHash, BrushEquals>;


	struct TextEngine::Impl {	

		CComPtr<ID3D11Device> device3d;
		
		/*
			Direct2D resources
		*/

		CComPtr<ID2D1Device> device;

		CComPtr<ID2D1Factory1> factory;

		CComPtr<ID2D1DeviceContext> context;

		CComPtr<ID2D1Bitmap1> target;

		/*
			DirectWrite resources
		*/
		CComPtr<IDWriteFactory> dWriteFactory;

		/*
			Custom font handling
		*/
		std::unique_ptr<FontLoader> fontLoader;

		eastl::vector<FontFile> fonts;

		CComPtr<IDWriteFontCollection> fontCollection;
		void LoadFontCollection();

		// Text format cache
		TextFormatsCache textFormats;
		IDWriteTextFormat* GetTextFormat(const TextStyle &textStyle);

		// Brush cache
		BrushCache brushes;
		ID2D1Brush* GetBrush(const Brush &brush);

		// Formatted strings
		CComPtr<IDWriteTextLayout> GetTextLayout(int width, int height, const TextStyle &style, const std::wstring &text);
		CComPtr<IDWriteTextLayout> GetTextLayout(int width, int height, const FormattedText &formatted, bool skipDrawingEffects = false);

		// Clipping
		bool enableClipRect = false;
		D2D1_RECT_F clipRect;

		void BeginDraw();
		void EndDraw();

	};
	
	class FontLoader : public IDWriteFontCollectionLoader, public IDWriteFontFileEnumerator, public IDWriteFontFileLoader {
	public:
		FontLoader(TextEngine::Impl &impl);

		HRESULT __declspec(nothrow) __stdcall CreateEnumeratorFromKey(
			IDWriteFactory* factory,
			void const* collectionKey,
			UINT32 collectionKeySize,
			IDWriteFontFileEnumerator** fontFileEnumerator
		) override;

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID uuid, void ** object) override;

		ULONG STDMETHODCALLTYPE AddRef() override { return 1; }

		ULONG STDMETHODCALLTYPE Release() override { return 1; }

		HRESULT STDMETHODCALLTYPE MoveNext(BOOL* hasCurrentFile) override;

		HRESULT STDMETHODCALLTYPE GetCurrentFontFile(IDWriteFontFile** currentFontFile) override;

		HRESULT STDMETHODCALLTYPE CreateStreamFromKey(
			void const* fontFileReferenceKey,
			uint32_t fontFileReferenceKeySize,
			IDWriteFontFileStream** fontFileStream
			) override;

	private:
		TextEngine::Impl& mImpl;

		size_t mStreamIndex = 0;
		CComPtr<IDWriteFontFile> mCurrentFontFile;
	};

	TextEngine::TextEngine(ID3D11Device* device3d, bool debugDevice)
		: mImpl(std::make_unique<Impl>()) {

		mImpl->device3d = device3d;

		// Create the D2D factory
		D2D1_FACTORY_OPTIONS options;
		if (debugDevice) {
			options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
			logger->info("Creating Direct2D Factory (debug=true).");
		} else {
			options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
			logger->info("Creating Direct2D Factory (debug=false).");
		}
		D3DVERIFY(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, &mImpl->factory));

		CComPtr<IDXGIDevice> dxgiDevice;
		D3DVERIFY(mImpl->device3d.QueryInterface(&dxgiDevice));

		// Create a D2D device on top of the DXGI device
		D3DVERIFY(mImpl->factory->CreateDevice(dxgiDevice, &mImpl->device));

		// Get Direct2D device's corresponding device context object.
		D3DVERIFY(mImpl->device->CreateDeviceContext(
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			&mImpl->context
		));

		// DirectWrite factory
		D3DVERIFY(DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&mImpl->dWriteFactory)
		));

		// Create our custom font handling objects
		mImpl->fontLoader = std::make_unique<FontLoader>(*mImpl);
		mImpl->dWriteFactory->RegisterFontCollectionLoader(mImpl->fontLoader.get());
		mImpl->dWriteFactory->RegisterFontFileLoader(mImpl->fontLoader.get());

		mImpl->context->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
	}

	TextEngine::~TextEngine() {
		mImpl->dWriteFactory->UnregisterFontFileLoader(mImpl->fontLoader.get());
		mImpl->dWriteFactory->UnregisterFontCollectionLoader(mImpl->fontLoader.get());		
	}

	void TextEngine::RenderText(const TigRect &rect, const FormattedText &formattedStr)
	{

		mImpl->BeginDraw();

		float x = (float) rect.x;
		float y = (float) rect.y;

		auto textLayout = mImpl->GetTextLayout(rect.width, rect.height, formattedStr);

		DWRITE_TEXT_METRICS metrics;
		D3DVERIFY(textLayout->GetMetrics(&metrics));

		// Draw the drop shadow first as a simple +1, +1 shift
		if (formattedStr.defaultStyle.dropShadow) {
			auto shadowLayout = mImpl->GetTextLayout(rect.width, rect.height, formattedStr, true);

			auto shadowBrush = mImpl->GetBrush(formattedStr.defaultStyle.dropShadowBrush);
			mImpl->context->DrawTextLayout(
				{ x + 1, y + 1 },
				shadowLayout,
				shadowBrush
			);
		}

		// Get applicable brush
		auto brush = mImpl->GetBrush(formattedStr.defaultStyle.foreground);

		// This is really unpleasant, but DirectWrite doesn't really use a well designed brush
		// coordinate system for drawing text apparently...
		CComPtr<ID2D1LinearGradientBrush> gradientBrush;
		if (SUCCEEDED(brush->QueryInterface(&gradientBrush))) {
			gradientBrush->SetStartPoint({ 0, y + metrics.top });
			gradientBrush->SetEndPoint({ 0, y + metrics.top + metrics.height });
		}

		mImpl->context->DrawTextLayout(
			{ x, y },
			textLayout,
			brush
		);

		// Draw an outlint
		/*D2D1_RECT_F rectOutline{
		(float)rect.x + metrics.left, (float)rect.y + metrics.top, (float)rect.x + metrics.left + metrics.widthIncludingTrailingWhitespace,
		(float)rect.y + metrics.top + metrics.height
		};
		mImpl->context->DrawRectangle(rectOutline, brush);*/
		
		mImpl->EndDraw();
	}

	void TextEngine::RenderTextRotated(const TigRect &rect, 
		float angle,
		XMFLOAT2 center,
		const FormattedText &formattedStr)
	{

		auto transform2d = D2D1::Matrix3x2F::Rotation(angle, { center.x, center.y });
		mImpl->context->SetTransform(transform2d);

		RenderText(rect, formattedStr);

		mImpl->context->SetTransform(D2D1::IdentityMatrix());

	}
		
	void TextEngine::RenderText(const TigRect &rect, const TextStyle & style, const std::wstring & text)
	{

		mImpl->BeginDraw();

		// Draw the drop shadow first as a simple +1, +1 shift
		if (style.dropShadow) {

			auto shadowLayout = mImpl->GetTextLayout(rect.width, rect.height, style, text);
			
			auto shadowBrush = mImpl->GetBrush(style.dropShadowBrush);
			mImpl->context->DrawTextLayout(
			{ (float)rect.x + 1, (float)rect.y + 1 },
				shadowLayout,
				shadowBrush
			);
		}

		auto textLayout = mImpl->GetTextLayout(rect.width, rect.height, style, text);

		DWRITE_TEXT_METRICS metrics;
		D3DVERIFY(textLayout->GetMetrics(&metrics));

		// Get applicable brush
		auto brush = mImpl->GetBrush(style.foreground);

		// This is really unpleasant, but DirectWrite doesn't really use a well designed brush
		// coordinate system for drawing text apparently...
		CComPtr<ID2D1LinearGradientBrush> gradientBrush;
		if (SUCCEEDED(brush->QueryInterface(&gradientBrush))) {			
			gradientBrush->SetStartPoint({ 0, (float) rect.y });
			gradientBrush->SetEndPoint({ 0, rect.y + metrics.height });
		}

		mImpl->context->DrawTextLayout(
		{ (float)rect.x + metrics.left, (float)rect.y + metrics.top },
			textLayout,
			brush
		);

		// Draw an outlint
		/*D2D1_RECT_F rectOutline{
			(float)rect.x + metrics.left, (float)rect.y + metrics.top, (float)rect.x + metrics.left + metrics.widthIncludingTrailingWhitespace,
				(float)rect.y + metrics.top + metrics.height
		};
		mImpl->context->DrawRectangle(rectOutline, brush);*/

		mImpl->EndDraw();
	}

	void TextEngine::RenderText(const TigRect &rect, const TextStyle & style, const std::string & text)
	{
		return RenderText(rect, style, local_to_ucs2(text));
	}

	void TextEngine::MeasureText(const FormattedText & formattedStr, TextMetrics & metrics)
	{
		auto textLayout = mImpl->GetTextLayout(metrics.width, metrics.height, formattedStr);

		uint32_t lineCount;
		DWRITE_LINE_METRICS lineMetrics;
		textLayout->GetLineMetrics(&lineMetrics, 1, &lineCount);

		DWRITE_TEXT_METRICS dWriteMetrics;
		D3DVERIFY(textLayout->GetMetrics(&dWriteMetrics));
		metrics.width = (int)ceilf(dWriteMetrics.widthIncludingTrailingWhitespace);
		metrics.height = (int)ceilf(dWriteMetrics.height);
		metrics.lineHeight = (int)roundf(lineMetrics.height);
		metrics.lines = lineCount;
	}

	void TextEngine::MeasureText(const TextStyle & style, const std::wstring& text, TextMetrics &metrics)
	{
		auto textLayout = mImpl->GetTextLayout(metrics.width, metrics.height, style, text);

		uint32_t lineCount;
		DWRITE_LINE_METRICS lineMetrics;
		textLayout->GetLineMetrics(&lineMetrics, 1, &lineCount);

		DWRITE_TEXT_METRICS dWriteMetrics;
		D3DVERIFY(textLayout->GetMetrics(&dWriteMetrics));
		metrics.width = (int)ceilf(dWriteMetrics.widthIncludingTrailingWhitespace);
		metrics.height = (int)ceilf(dWriteMetrics.height);
		metrics.lineHeight = (int)roundf(lineMetrics.height);
		metrics.lines = lineCount;
	}

	void TextEngine::MeasureText(const TextStyle & style, const std::string& text, TextMetrics &metrics)
	{
		MeasureText(style, local_to_ucs2(text), metrics);
	}

	void TextEngine::SetRenderTarget(ID3D11Texture2D * renderTarget)
	{
		mImpl->target.Release();

		if (!renderTarget) {
			mImpl->context->SetTarget(nullptr);
			return;
		}

		// Get the underlying DXGI surface
		CComPtr<IDXGISurface> dxgiSurface;
		D3DVERIFY(renderTarget->QueryInterface(&dxgiSurface));

		D3D11_TEXTURE2D_DESC rtDesc;
		renderTarget->GetDesc(&rtDesc);

		// Create a D2D RT bitmap for it
		D2D1_BITMAP_PROPERTIES1 bitmapProperties{
			{ DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_IGNORE},
			96.0f,
			96.0f,
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		};
		
		D3DVERIFY(mImpl->context->CreateBitmapFromDxgiSurface(
			dxgiSurface,
			&bitmapProperties,
			&mImpl->target
		));

		mImpl->context->SetTarget(mImpl->target);
	}

	void TextEngine::AddFont(const std::string &filename)
	{
		FontFile file;
		file.data = std::move(vfs->ReadAsBinary(filename));
		file.filename = filename;

		mImpl->fonts.emplace_back(std::move(file));
		mImpl->fontCollection.Release(); // Has to be rebuilt...
		mImpl->textFormats.clear();
	}

	bool TextEngine::HasFontFamily(const std::string & name)
	{
		if (!mImpl->fontCollection) {
			mImpl->LoadFontCollection();
		}

		auto requested_font_family = utf8_to_ucs2(name);
		UINT index;
		BOOL exists;
		auto result = mImpl->fontCollection->FindFamilyName(requested_font_family.c_str(), &index, &exists);
		if (!SUCCEEDED(result)) {
			logger->warn("Unable to query font collection for family '{}': {:x}", name, result);
			return false;
		}

		return exists == TRUE;

	}

	void TextEngine::SetScissorRect(const TigRect & rect)
	{
		mImpl->enableClipRect = true;
		mImpl->clipRect.left = (float) rect.x;
		mImpl->clipRect.top = (float) rect.y;
		mImpl->clipRect.right = (float) rect.x + rect.width;
		mImpl->clipRect.bottom = (float) rect.y + rect.height;
	}

	void TextEngine::ResetScissorRect()
	{
		mImpl->enableClipRect = false;
	}

	IDWriteTextFormat* TextEngine::Impl::GetTextFormat(const TextStyle &textStyle)
	{
		auto it = textFormats.find(textStyle);
		if (it != textFormats.end()) {
			return it->second;
		}
		
		auto fontWeight = DWRITE_FONT_WEIGHT_NORMAL;
		auto fontStyle = DWRITE_FONT_STYLE_NORMAL;

		if (textStyle.bold) {
			fontWeight = DWRITE_FONT_WEIGHT_BOLD;
		}
		if (textStyle.italic) {
			fontStyle = DWRITE_FONT_STYLE_ITALIC;
		}

		CComPtr<IDWriteTextFormat> textFormat;

		// Lazily build the font collection
		if (!fontCollection) {
			LoadFontCollection();
		}

		// Lazily create the text format
		D3DVERIFY(dWriteFactory->CreateTextFormat(
			utf8_to_ucs2(textStyle.fontFace.c_str()).c_str(),
			fontCollection,
			fontWeight,
			fontStyle,
			DWRITE_FONT_STRETCH_NORMAL,
			textStyle.pointSize,
			L"",
			&textFormat
		));

		if (textStyle.tabStopWidth > 0) {
			D3DVERIFY(textFormat->SetIncrementalTabStop(textStyle.tabStopWidth));
		}

		D3DVERIFY(textFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP));

		// D3DVERIFY(textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
		
		switch (textStyle.align) {
		case TextAlign::Left:
			D3DVERIFY(textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
			break;
		case TextAlign::Center:
			D3DVERIFY(textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
			break;
		case TextAlign::Right:
			D3DVERIFY(textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING));
			break;
		case TextAlign::Justified:
			D3DVERIFY(textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED));
			break;
		}

		switch (textStyle.paragraphAlign) {
		case ParagraphAlign::Near:
			D3DVERIFY(textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
			break;
		case ParagraphAlign::Far:
			D3DVERIFY(textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR));
			break;
		case ParagraphAlign::Center:
			D3DVERIFY(textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
			break;
		}

		if (textStyle.uniformLineHeight) {
			D3DVERIFY(textFormat->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, 
				textStyle.lineHeight, 
				textStyle.baseLine));
		}
		
		textFormats.insert({ textStyle, textFormat });
		return textFormat;

	}

	D2D1_COLOR_F ToD2dColor(XMCOLOR color) {
		D2D1_COLOR_F result;
		result.a = color.a / 255.0f;
		result.r = color.r / 255.0f;
		result.g = color.g / 255.0f;
		result.b = color.b / 255.0f;
		return result;
	}

	ID2D1Brush * TextEngine::Impl::GetBrush(const Brush & brush)
	{
		auto it = brushes.find(brush);

		if (it != brushes.end()) {
			return it->second;
		}

		CComPtr<ID2D1Brush> simpleBrush;

		if (!brush.gradient) {
			D2D1_COLOR_F colorValue = ToD2dColor(brush.primaryColor);
			CComPtr<ID2D1SolidColorBrush> brushResource;
			D3DVERIFY(context->CreateSolidColorBrush(colorValue, &brushResource));
			D3DVERIFY(brushResource.QueryInterface(&simpleBrush));
		} else {

			D2D1_GRADIENT_STOP gradientStops[2];
			gradientStops[0].color = ToD2dColor(brush.primaryColor);
			gradientStops[0].position = 0;
			gradientStops[1].color = ToD2dColor(brush.secondaryColor);
			gradientStops[1].position = 1;
			
			// Create the gradient stops
			CComPtr<ID2D1GradientStopCollection> gradientStopColl;
			D3DVERIFY(context->CreateGradientStopCollection(gradientStops, 2, &gradientStopColl));

			// Configure the gradient to go top->down
			D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES gradientProps;
			gradientProps.startPoint = { 0, 0 };
			gradientProps.endPoint = { 0, 1 };

			CComPtr<ID2D1LinearGradientBrush> brushResource;
			D3DVERIFY(context->CreateLinearGradientBrush(gradientProps, gradientStopColl, &brushResource));
			D3DVERIFY(brushResource.QueryInterface(&simpleBrush));
		}
		
		brushes.insert({ brush, simpleBrush });
		return simpleBrush;
	}
	
	CComPtr<IDWriteTextLayout> TextEngine::Impl::GetTextLayout(int width, int height, const TextStyle & style, const std::wstring & text)
	{
		// The maximum width/height of the box may or may not be specified
		float widthF, heightF;
		if (width > 0) {
			widthF = (float)width;
		} else {
			widthF = std::numeric_limits<float>::max();
		}
		if (height > 0) {
			heightF = (float)height;
		}
		else {
			heightF = std::numeric_limits<float>::max();
		}

		auto textFormat = GetTextFormat(style);

		CComPtr<IDWriteTextLayout> textLayout;
		D3DVERIFY(dWriteFactory->CreateTextLayout(
			text.c_str(),
			text.size(),
			textFormat,
			widthF,
			heightF,
			&textLayout
		));

		if (style.trim) {
			// Ellipsis for the end of the str
			CComPtr<IDWriteInlineObject> trimmingSign;
			D3DVERIFY(dWriteFactory->CreateEllipsisTrimmingSign(textFormat, &trimmingSign));

			DWRITE_TRIMMING trimming;
			trimming.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
			trimming.delimiter = 0;
			trimming.delimiterCount = 0;
			textLayout->SetTrimming(&trimming, trimmingSign);
		}

		return textLayout;
	}

	CComPtr<IDWriteTextLayout> TextEngine::Impl::GetTextLayout(int width, int height, const FormattedText & formatted, bool skipDrawingEffects)
	{
		// The maximum width/height of the box may or may not be specified
		float widthF, heightF;
		if (width > 0) {
			widthF = (float)width;
		}
		else {
			widthF = std::numeric_limits<float>::max();
		}
		if (height > 0) {
			heightF = (float)height;
		}
		else {
			heightF = std::numeric_limits<float>::max();
		}

		auto textFormat = GetTextFormat(formatted.defaultStyle);
		auto &text = formatted.text;

		CComPtr<IDWriteTextLayout> textLayout;
		D3DVERIFY(dWriteFactory->CreateTextLayout(
			text.c_str(),
			text.size(),
			textFormat,
			widthF,
			heightF,
			&textLayout
		));

		if (formatted.defaultStyle.trim) {
			// Ellipsis for the end of the str
			CComPtr<IDWriteInlineObject> trimmingSign;
			D3DVERIFY(dWriteFactory->CreateEllipsisTrimmingSign(textFormat, &trimmingSign));

			DWRITE_TRIMMING trimming;
			trimming.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
			trimming.delimiter = 0;
			trimming.delimiterCount = 0;
			textLayout->SetTrimming(&trimming, trimmingSign);
		}

		for (auto &range : formatted.formats) {
			DWRITE_TEXT_RANGE textRange;
			textRange.startPosition = range.startChar;
			textRange.length = range.length;

			// This will collide with drop shadow drawing for example
			if (!skipDrawingEffects) {
				auto rangeBrush = GetBrush(range.style.foreground);
				textLayout->SetDrawingEffect(rangeBrush, textRange);
			}
		}

		return textLayout;

	}

	void TextEngine::Impl::BeginDraw()
	{
		context->BeginDraw();

		if (enableClipRect) {
			context->PushAxisAlignedClip(clipRect, D2D1_ANTIALIAS_MODE_ALIASED);
		}		
	}

	void TextEngine::Impl::EndDraw()
	{
		if (enableClipRect) {
			context->PopAxisAlignedClip();
		}

		D3DVERIFY(context->EndDraw());
	}

	FontLoader::FontLoader(TextEngine::Impl &impl) : mImpl(impl) {
	}

	HRESULT FontLoader::CreateEnumeratorFromKey(IDWriteFactory * factory, void const * collectionKey, UINT32 collectionKeySize, IDWriteFontFileEnumerator ** fontFileEnumerator)
	{
		*fontFileEnumerator = this;
		mStreamIndex = 0;
		return S_OK;
	}

	HRESULT FontLoader::MoveNext(BOOL* hasCurrentFile)
	{
		*hasCurrentFile = FALSE;

		if (mStreamIndex < mImpl.fonts.size())
		{
			mCurrentFontFile.Release();
			D3DVERIFY(mImpl.dWriteFactory->CreateCustomFontFileReference(
				&mStreamIndex,
				sizeof(mStreamIndex),
				this,
				&mCurrentFontFile
			));

			*hasCurrentFile = TRUE;
			++mStreamIndex;
		}

		return S_OK;
	}

	HRESULT FontLoader::GetCurrentFontFile(IDWriteFontFile** currentFontFile)
	{
		// This is effectively just an AddRef
		*currentFontFile = CComPtr<IDWriteFontFile>(mCurrentFontFile).Detach();
		return S_OK;
	}

	HRESULT FontLoader::CreateStreamFromKey(void const * fontFileReferenceKey, 
		uint32_t fontFileReferenceKeySize, 
		IDWriteFontFileStream ** fontFileStream)
	{		
		assert(fontFileReferenceKeySize == sizeof(size_t));
		size_t index = *(size_t*)fontFileReferenceKey;

		auto &font = mImpl.fonts[index];
		*fontFileStream = new MemoryFontStream(font.data);
		return S_OK;
	}

	HRESULT FontLoader::QueryInterface(REFIID uuid, void ** object)
	{
		if (uuid == IID_IUnknown
			|| uuid == __uuidof(IDWriteFontCollectionLoader)
			|| uuid == __uuidof(IDWriteFontFileEnumerator)
			|| uuid == __uuidof(IDWriteFontFileLoader)
			)
		{
			*object = this;
			return S_OK;
		}
		else
		{
			*object = nullptr;
			return E_NOINTERFACE;
		}
	}

	void TextEngine::Impl::LoadFontCollection()
	{
		// DirectWrite caches font collections internally.
		// If we reload, we need to generate a new key
		static uint32_t fontCollKey = 1;

		logger->info("Reloading font collection...");
		D3DVERIFY(dWriteFactory->CreateCustomFontCollection(
			fontLoader.get(),
			&fontCollKey,
			sizeof(fontCollKey),
			&fontCollection
		));
		fontCollKey++;

		// Enumerate all loaded fonts to the logger
		auto count = fontCollection->GetFontFamilyCount();
		for (auto i = 0u; i < count; i++) {
			CComPtr<IDWriteFontFamily> family;
			D3DVERIFY(fontCollection->GetFontFamily(i, &family));

			CComPtr<IDWriteLocalizedStrings> familyNames;
			D3DVERIFY(family->GetFamilyNames(&familyNames));
			std::string familyNamesList;
			for (auto j = 0u; j < familyNames->GetCount(); j++) {
				std::wstring nameBuffer;
				uint32_t nameLength;
				D3DVERIFY(familyNames->GetStringLength(j, &nameLength));
				nameBuffer.resize(nameLength + 1);
				D3DVERIFY(familyNames->GetString(j, &nameBuffer[0], nameLength + 1));
				nameBuffer.resize(nameLength); // Remove the null-byte
				if (!familyNamesList.empty()) {
					familyNamesList.append(", ");
				}
				familyNamesList.append(ucs2_to_utf8(nameBuffer));
			}
			logger->info(" Loaded Font Family: {}", familyNamesList);
		}
	}

	HRESULT MemoryFontStream::ReadFileFragment(void const ** fragmentStart, uint64_t fileOffset, uint64_t fragmentSize, void ** fragmentContext)
	{
		if (fileOffset > mData.size() || fragmentSize > mData.size() - fileOffset) {
			*fragmentStart = nullptr;
			*fragmentContext = nullptr;
			return E_FAIL;
		}
		
		*fragmentContext = nullptr;
		*fragmentStart = mData.data() + fileOffset;
		return S_OK;
	}

	void MemoryFontStream::ReleaseFileFragment(void * fragmentContext)
	{
		// No concept of freeing the const memory in mData
	}

	HRESULT MemoryFontStream::GetFileSize(uint64_t * fileSize)
	{
		*fileSize = mData.size();
		return S_OK;
	}

	HRESULT MemoryFontStream::GetLastWriteTime(uint64_t * lastWriteTime)
	{
		return E_NOTIMPL;
	}

	HRESULT MemoryFontStream::QueryInterface(REFIID uuid, LPVOID * objOut)
	{
		if (!objOut) {
			return E_INVALIDARG;
		}
		*objOut = nullptr;
		if (uuid == IID_IUnknown || uuid == __uuidof(IDWriteFontFileStream)) {
			*objOut = this;
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	ULONG MemoryFontStream::AddRef()
	{
		return InterlockedIncrement(&mRefCount);
	}

	ULONG MemoryFontStream::Release()
	{
		ULONG refCount = InterlockedDecrement(&mRefCount);
		if (!refCount) {
			delete this;
		}
		return refCount;
	}

}
