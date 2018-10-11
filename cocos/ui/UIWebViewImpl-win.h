#ifndef __COCOS2D__UI__WEBVIEWIMPL_WINDOWS_H_
#define __COCOS2D__UI__WEBVIEWIMPL_WINDOWS_H_

#pragma once

#ifdef _WINDOWS

#include <stdlib.h>
#include <string>

#include "platform/CCStdC.h"

#if _MSC_VER >= 1600 || defined(__MINGW32__)
#include <stdint.h>
#else
#include "../platform/win32/compat/stdint.h"
#endif

namespace cocos2d {
	class Data;
	class Renderer;
	class Mat4;

	namespace experimental {
		namespace ui{
			class WebView;
		}
	}
}

class Win32WebControl;

namespace cocos2d {
	namespace experimental {
		namespace ui {

            class WebViewImpl
            {
            public:
                WebViewImpl(cocos2d::experimental::ui::WebView *webView);
                virtual ~WebViewImpl();

				void setJavascriptInterfaceScheme(const std::string &scheme);

				void loadData(const cocos2d::Data &data, const std::string &MIMEType, const std::string &encoding, const std::string &baseURL);

				void loadHTMLString(const std::string &string, const std::string &baseURL);

				void loadURL(const std::string &url);

				void loadURL(const std::string& url, bool cleanCachedData);

				void loadFile(const std::string &fileName);

				void stopLoading();

				void reload();

				bool canGoBack();

				bool canGoForward();

				void goBack();

				void goForward();

				void evaluateJS(const std::string &js);

				void setScalesPageToFit(const bool scalesPageToFit);
				void setOpacityWebView(float opacity);

				float getOpacityWebView()const;

				void setBackgroundTransparent();

				virtual void draw(cocos2d::Renderer *renderer, cocos2d::Mat4 const &transform, uint32_t flags);

				virtual void setVisible(bool visible);

				void setBounces(bool bounces);

            private:
                bool _createSucceeded;
                Win32WebControl *_systemWebControl;
                WebView *_webView;
            };
        } // namespace ui
    } // namespace experimental
} //cocos2d

#endif

#endif /* __COCOS2D__UI__WEBVIEWIMPL_WINDOWS_H_ */