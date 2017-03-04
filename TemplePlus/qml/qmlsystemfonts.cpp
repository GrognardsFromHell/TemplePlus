
#include "qmlsystemfonts.h"

#include <QDebug>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QFont>

#include <dwrite.h>
#include <atlcomcli.h>

void InstallQmlSystemFonts()
{
	
	CComPtr<IDWriteFactory> factory;

	HRESULT hr;

	// Create the DWrite factory
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&factory));
	if (!SUCCEEDED(hr)) {
		qErrnoWarning(hr, "Unable to create DirectWrite factory.");
		return;
	}

	CComPtr<IDWriteFontCollection> systemFonts;
	hr = factory->GetSystemFontCollection(&systemFonts);
	if (!SUCCEEDED(hr)) {
		qErrnoWarning(hr, "Unable to retrieve system font collection.");
		return;
	}

	BOOL exists = FALSE;
	UINT32 index;
	hr = systemFonts->FindFamilyName(L"Arial", &index, &exists);
	if (!SUCCEEDED(hr)) {
		qErrnoWarning(hr, "Failed to search for Arial in font system collection.");
		return;
	}
	if (!exists) {
		qWarning("System is missing Arial font!");
		return;
	}

	CComPtr<IDWriteFontFamily> arialFamily;
	hr = systemFonts->GetFontFamily(index, &arialFamily);
	if (FAILED(hr)) {
		qErrnoWarning(hr, "Failed to get Arial font family from system font collection.");
	}

	auto fontCount = arialFamily->GetFontCount();
	for (auto i = 0u; i < fontCount; i++) {
		CComPtr<IDWriteFont> arialFont;
		hr = arialFamily->GetFont(i, &arialFont);
		if (FAILED(hr)) {
			qErrnoWarning(hr, "Unable to get font index %d from arial font family.", i);
			continue;
		}

		CComPtr<IDWriteFontFace> arialFontFace;
		hr = arialFont->CreateFontFace(&arialFontFace);
		if (FAILED(hr)) {
			qErrnoWarning(hr, "Unable to create the font face corresponding to the arial font @ index %d", i);
			continue;
		}

		UINT32 numberOfFiles;
		hr = arialFontFace->GetFiles(&numberOfFiles, nullptr);
		if (FAILED(hr)) {
			qErrnoWarning(hr, "Unable to get the number of font files for font index %d", i);
			continue;
		}

		std::vector<CComPtr<IDWriteFontFile>> fontFiles;
		fontFiles.resize(numberOfFiles);

		hr = arialFontFace->GetFiles(&numberOfFiles, &fontFiles[0]);
		if (FAILED(hr)) {
			qErrnoWarning(hr, "Unable to retrieve the font files for font index %d", i);
			continue;
		}

		for (auto &fontFile : fontFiles) {
			CComPtr<IDWriteFontFileLoader> loader;
			hr = fontFile->GetLoader(&loader);
			if (FAILED(hr)) {
				qErrnoWarning(hr, "Unable to retrieve font file loader");
				continue;
			}

			// Should be a local file...
			CComPtr<IDWriteLocalFontFileLoader> localLoader;
			hr = loader.QueryInterface(&localLoader);
			if (FAILED(hr)) {
				qErrnoWarning(hr, "Loader for font face is not a local file loader.");
				continue;
			}

			const void* fileKey;
			UINT32 fileKeyLength;
			hr = fontFile->GetReferenceKey(&fileKey, &fileKeyLength);
			if (FAILED(hr)) {
				qErrnoWarning(hr, "Unable to retrieve the font file key.");
				continue;
			}

			wchar_t path[MAX_PATH];
			hr = localLoader->GetFilePathFromKey(fileKey, fileKeyLength, &path[0], MAX_PATH);
			if (FAILED(hr)) {
				qErrnoWarning(hr, "Unable to retrieve the path for a font file.");
				continue;
			}

			QString fontPath = QString::fromWCharArray(path);
			qDebug() << "Loading system font" << fontPath;
			QFontDatabase::addApplicationFont(fontPath);
		}
	}
	
	QFont font("Arial", 11);
	QGuiApplication::setFont(font);

}
