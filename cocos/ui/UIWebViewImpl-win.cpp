#ifdef _WINDOWS

#include "UIWebViewImpl-win.h"

#include "UIWebView.h"
#include "platform/CCGLView.h"
#include "base/CCDirector.h"
#include "platform/CCFileUtils.h"

namespace cocos2d {
	namespace experimental {
		namespace ui{
			
			WebViewImpl::WebViewImpl(WebView *webView)
			{
			}
			
			WebViewImpl::~WebViewImpl()
			{
			}

			void WebViewImpl::setJavascriptInterfaceScheme(const std::string &scheme)
			{
			}

			void WebViewImpl::loadData(const cocos2d::Data &data, const std::string &MIMEType, const std::string &encoding, const std::string &baseURL)
			{

			}

			void WebViewImpl::loadHTMLString(const std::string &string, const std::string &baseURL)
			{
			}

			void WebViewImpl::loadURL(const std::string &url)
			{
			}

			void WebViewImpl::loadFile(const std::string &fileName)
			{
			}

			void WebViewImpl::stopLoading()
			{
			}

			void WebViewImpl::reload()
			{
			}

			bool WebViewImpl::canGoBack()
			{
				return false;
			}

			bool WebViewImpl::canGoForward()
			{
				return false;
			}

			void WebViewImpl::goBack()
			{
			}

			void WebViewImpl::goForward()
			{
			}

			void WebViewImpl::evaluateJS(const std::string &js)
			{
			}

			void WebViewImpl::setScalesPageToFit(const bool scalesPageToFit)
			{
			}

			void WebViewImpl::draw(cocos2d::Renderer *renderer, cocos2d::Mat4 const &transform, uint32_t flags)
			{
			}

			void WebViewImpl::setVisible(bool visible)
			{
			}

		} // namespace ui
	} // namespace experimental
} //namespace cocos2d

#endif