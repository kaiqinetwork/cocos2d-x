/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2014 Chukong Technologies Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "2d/CCTextFieldTTF.h"

#include "base/CCDirector.h"
#include "2d/CCSprite.h"
#include "base/ccUTF8.h"
#include "2d/CCActionInterval.h"
#include "base/CCEventMouse.h"
#include "glfw3/glfw3.h"

NS_CC_BEGIN

static int _calcCharCount(const char * text)
{
    int n = 0;
    char ch = 0;
    while ((ch = *text))
    {
        CC_BREAK_IF(! ch);

        if (0x80 != (0xC0 & ch))
        {
            ++n;
        }
        ++text;
    }
    return n;
}

static int _calcCharPos(const char * text, int charCount)
{
	int n = 0;
	char ch = 0;
	const char* beginPtr = text;
	while ((ch = *text))
	{
		CC_BREAK_IF(!ch);

		if (0x80 != (0xC0 & ch))
		{
			++n;
			if ((n - 1) == charCount)
			{
				break;
			}
		}
		++text;
	}
	return text - beginPtr;
}

//////////////////////////////////////////////////////////////////////////
// constructor and destructor
//////////////////////////////////////////////////////////////////////////

TextFieldTTF::TextFieldTTF()
: _delegate(0)
, _charCount(0)
, _inputText("")
, _placeHolder("")   // prevent Label initWithString assertion
, _colorText(Color4B::WHITE)
, _secureTextEntry(false)
, _blockNode(nullptr)
, _blockTextSprite(nullptr)
, _cursorSprite(nullptr)
, _blockStart(0)
, _blockEnd(0)
, _cursorPos(0)
{
    _colorSpaceHolder.r = _colorSpaceHolder.g = _colorSpaceHolder.b = 127;
    _colorSpaceHolder.a = 255;
}

TextFieldTTF::~TextFieldTTF()
{
}

//////////////////////////////////////////////////////////////////////////
// static constructor
//////////////////////////////////////////////////////////////////////////

TextFieldTTF * TextFieldTTF::textFieldWithPlaceHolder(const std::string& placeholder, const Size& dimensions, TextHAlignment alignment, const std::string& fontName, float fontSize)
{
    TextFieldTTF *ret = new (std::nothrow) TextFieldTTF();
    if(ret && ret->initWithPlaceHolder("", dimensions, alignment, fontName, fontSize))
    {
        ret->autorelease();
        if (placeholder.size()>0)
        {
            ret->setPlaceHolder(placeholder);
        }
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

TextFieldTTF * TextFieldTTF::textFieldWithPlaceHolder(const std::string& placeholder, const std::string& fontName, float fontSize)
{
    TextFieldTTF *ret = new (std::nothrow) TextFieldTTF();
    if(ret && ret->initWithPlaceHolder("", fontName, fontSize))
    {
        ret->autorelease();
        if (placeholder.size()>0)
        {
            ret->setPlaceHolder(placeholder);
        }
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// initialize
//////////////////////////////////////////////////////////////////////////

bool TextFieldTTF::initWithPlaceHolder(const std::string& placeholder, const Size& dimensions, TextHAlignment alignment, const std::string& fontName, float fontSize)
{
    _placeHolder = placeholder;
    setDimensions(dimensions.width,dimensions.height);
    setSystemFontName(fontName);
    setSystemFontSize(fontSize);
    setAlignment(alignment,TextVAlignment::CENTER);
    Label::setTextColor(_colorSpaceHolder);
    Label::setString(_placeHolder);

    return true;
}
bool TextFieldTTF::initWithPlaceHolder(const std::string& placeholder, const std::string& fontName, float fontSize)
{
    _placeHolder = std::string(placeholder);
    setSystemFontName(fontName);
    setSystemFontSize(fontSize);
    Label::setTextColor(_colorSpaceHolder);
    Label::setString(_placeHolder);

    return true;
}

//////////////////////////////////////////////////////////////////////////
// IMEDelegate
//////////////////////////////////////////////////////////////////////////

bool TextFieldTTF::attachWithIME()
{
    bool ret = IMEDelegate::attachWithIME();
    if (ret)
    {
        // open keyboard
        auto pGlView = Director::getInstance()->getOpenGLView();
        if (pGlView)
        {
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WP8 && CC_TARGET_PLATFORM != CC_PLATFORM_WINRT)
            pGlView->setIMEKeyboardState(true);
#else
            pGlView->setIMEKeyboardState(true, _inputText);
#endif
        }
    }
    return ret;
}

bool TextFieldTTF::detachWithIME()
{
    bool ret = IMEDelegate::detachWithIME();
    if (ret)
    {
        // close keyboard
        auto glView = Director::getInstance()->getOpenGLView();
        if (glView)
        {
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WP8 && CC_TARGET_PLATFORM != CC_PLATFORM_WINRT)
            glView->setIMEKeyboardState(false);
#else
            glView->setIMEKeyboardState(false, "");
#endif
        }
    }
    return ret;
}

bool TextFieldTTF::canAttachWithIME()
{
    return (_delegate) ? (! _delegate->onTextFieldAttachWithIME(this)) : true;
}

bool TextFieldTTF::canDetachWithIME()
{
    return (_delegate) ? (! _delegate->onTextFieldDetachWithIME(this)) : true;
}

void TextFieldTTF::insertText(const char * text, size_t len)
{
    std::string insert(text, len);

    // insert \n means input end
    int pos = static_cast<int>(insert.find('\n'));
    if ((int)insert.npos != pos)
    {
        len = pos;
        insert.erase(pos);
    }

    if (len > 0)
    {
        if (_delegate && _delegate->onTextFieldInsertText(this, insert.c_str(), len))
        {
            // delegate doesn't want to insert text
            return;
        }

		int charCount = _calcCharCount(insert.c_str());
        std::string sText(_inputText);
		if (_blockEnd > _blockStart)
		{
			int startPos = _calcCharPos(sText.c_str(), _blockStart);
			int endPos = _calcCharPos(sText.c_str(), _blockEnd);
			sText.replace(startPos, endPos - startPos, insert);
			_cursorPos = _blockStart + charCount;
			_blockStart = _blockEnd = 0;
		}
		else
		{
			sText.insert(_cursorPos == _charCount ? sText.size() : _calcCharPos(sText.c_str(), _cursorPos), insert);
			_cursorPos += charCount;
		}
        _setString(sText);
    }

    if ((int)insert.npos == pos) {
        return;
    }

    // '\n' inserted, let delegate process first
    if (_delegate && _delegate->onTextFieldInsertText(this, "\n", 1))
    {
        return;
    }

    // if delegate hasn't processed, detach from IME by default
    detachWithIME();
}

void TextFieldTTF::deleteBackward()
{
    size_t len = _inputText.length();
	if (!len || (_cursorPos == 0 && _blockEnd == _blockStart))
    {
        // there is no string
        return;
    }

    // get the delete byte number
    size_t deleteLen = 1;    // default, erase 1 byte

	if (_blockEnd > _blockStart)
	{
		_cursorPos = _blockEnd;
	}

	size_t pos = _calcCharPos(_inputText.c_str(), _cursorPos);

	if (_blockEnd > _blockStart)
	{
		deleteLen = pos - _calcCharPos(_inputText.c_str(), _blockStart);
		_cursorPos = _blockStart;
		_blockStart = 0;
		_blockEnd = 0;
	}
	else
	{
		while (0x80 == (0xC0 & _inputText.at(pos - deleteLen)))
		{
			++deleteLen;
		}
		_cursorPos -= 1;
	}

    if (_delegate && _delegate->onTextFieldDeleteBackward(this, _inputText.c_str() + len - deleteLen, static_cast<int>(deleteLen)))
    {
        // delegate doesn't want to delete backwards
        return;
    }

    // if all text deleted, show placeholder string
    if (len <= deleteLen)
    {
        _inputText = "";
        _charCount = 0;
		_cursorPos = 0;
        Label::setTextColor(_colorSpaceHolder);
        Label::setString(_placeHolder);
        return;
    }

    // set new input text
    std::string text(_inputText.c_str());
	text.erase(pos - deleteLen, deleteLen);
    _setString(text);
}

const std::string& TextFieldTTF::getContentText()
{
    return _inputText;
}

void TextFieldTTF::setTextColor(const Color4B &color)
{
    _colorText = color;
    if (!_inputText.empty()) {
        Label::setTextColor(_colorText);
    }
}

void TextFieldTTF::visit(Renderer *renderer, const Mat4 &parentTransform, uint32_t parentFlags)
{
    if (_delegate && _delegate->onVisit(this,renderer,parentTransform,parentFlags))
    {
        return;
    }
    Label::visit(renderer,parentTransform,parentFlags);
}

const Color4B& TextFieldTTF::getColorSpaceHolder()
{
    return _colorSpaceHolder;
}

void TextFieldTTF::setColorSpaceHolder(const Color3B& color)
{
    _colorSpaceHolder.r = color.r;
    _colorSpaceHolder.g = color.g;
    _colorSpaceHolder.b = color.b;
    _colorSpaceHolder.a = 255;
    if (_inputText.empty())
    {
        Label::setTextColor(_colorSpaceHolder);
    }
}

void TextFieldTTF::setColorSpaceHolder(const Color4B& color)
{
    _colorSpaceHolder = color;
    if (_inputText.empty()) {
        Label::setTextColor(_colorSpaceHolder);
    }
}

//////////////////////////////////////////////////////////////////////////
// properties
//////////////////////////////////////////////////////////////////////////

// input text property
void TextFieldTTF::setString(const std::string &text)
{
    _setString(text);
	_cursorPos = _charCount;
}

const std::string& TextFieldTTF::getString() const
{
    return _inputText;
}

// place holder text property
void TextFieldTTF::setPlaceHolder(const std::string& text)
{
    _placeHolder = text;
    if (_inputText.empty())
    {
        Label::setTextColor(_colorSpaceHolder);
        Label::setString(_placeHolder);
    }
}

const std::string& TextFieldTTF::getPlaceHolder() const
{
    return _placeHolder;
}

// secureTextEntry
void TextFieldTTF::setSecureTextEntry(bool value)
{
    if (_secureTextEntry != value)
    {
        _secureTextEntry = value;
        setString(_inputText);
    }
}

bool TextFieldTTF::isSecureTextEntry()
{
    return _secureTextEntry;
}

void TextFieldTTF::updateContent()
{
	if (_systemFontDirty)
	{
		_systemFontDirty = false;
	}

	if (_textSprite)
	{
		Node::removeChild(_textSprite, true);
		CC_SAFE_RELEASE_NULL(_textSprite);
		_blockNode = nullptr;
		_blockTextSprite = nullptr;
		_cursorSprite = nullptr;
	}
	CC_SAFE_RELEASE_NULL(_shadowNode);

	FontDefinition fd = _getFontDefinition();
	if (_shadowEnabled)
	{
		createShadowSpriteForSystemFont(fd);
	}

	_currentLabelType = LabelType::STRING_TEXTURE;

	fd._dimensions = Size::ZERO;
	fd._alignment = TextHAlignment::LEFT;
	Size textSize = Texture2D::getContentSizeWithString(_utf16Text.c_str(), fd);

	Size contentSize;
	contentSize.width = _labelWidth;
	contentSize.height = _labelHeight;
	this->setContentSize(contentSize);

	std::u16string tempText;
	Size tempSize;

	float cursorOffset = 0;
	if (_cursorPos > 0)
	{
		tempText = _utf16Text.substr(0, _cursorPos);
		cursorOffset = Texture2D::getContentSizeWithString(tempText.c_str(), fd).width;
	}
	else
		cursorOffset = 0;

	Rect textureRect;
	if (contentSize.width > textSize.width)
	{
		switch (fd._alignment)
		{
		case TextHAlignment::LEFT:
			_textOffset.x = 0;
			break;
		case TextHAlignment::RIGHT:
			_textOffset.x = (contentSize.width - textSize.width);
			break;
		case TextHAlignment::CENTER:
			_textOffset.x = ((contentSize.width - textSize.width) / 2);
			break;
		default:
			break;
		}
		textureRect.size = textSize;
	}
	else
	{
		if (cursorOffset > contentSize.width)
		{
			_textOffset.x = contentSize.width - cursorOffset;
		}
		else if ((cursorOffset + _textOffset.x) < 0)
		{
			_textOffset.x = -cursorOffset;
		}
		textureRect.origin.x = -_textOffset.x;
		textureRect.origin.y = 0;
		textureRect.size.width = contentSize.width;
		textureRect.size.height = textSize.height;
	}
	switch (fd._vertAlignment)
	{
	case TextVAlignment::TOP:
		_textOffset.y = contentSize.height - textSize.height;
		break;
	case TextVAlignment::CENTER:
		_textOffset.y = (contentSize.height - textSize.height) / 2;
		break;
	case TextVAlignment::BOTTOM:
		_textOffset.y = 0;
		break;
	default:
		break;
	}

	auto texture = new (std::nothrow) Texture2D;
	texture->initWithString(_utf8Text.c_str(), fd);
	
	_textSprite = Sprite::createWithTexture(texture, textureRect);
	//set camera mask using label's camera mask, because _textSprite may be null when setting camera mask to label
	_textSprite->setCameraMask(getCameraMask());
	_textSprite->setGlobalZOrder(getGlobalZOrder());
	_textSprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	_textSprite->setPosition(std::fmax(_textOffset.x, 0), _textOffset.y);
	texture->release();
	if (_blendFuncDirty)
	{
		_textSprite->setBlendFunc(_blendFunc);
	}

	if (_blockStart > _utf16Text.size() || _blockEnd > _utf16Text.size() || _blockStart > _blockEnd)
	{
		_blockStart = _blockEnd = 0;
	}

	std::u16string blockText16 = _utf16Text.substr(_blockStart, _blockEnd - _blockStart);
	if (!blockText16.empty())
	{
		Rect blockRect;

		if (_blockStart > 0)
		{
			tempText = _utf16Text.substr(0, _blockStart);
			tempSize = Texture2D::getContentSizeWithString(tempText.c_str(), fd);
			blockRect.origin.x = tempSize.width;
		}

		tempSize = Texture2D::getContentSizeWithString(blockText16.c_str(), fd);
		blockRect.size.width = tempSize.width;
		blockRect.size.height = tempSize.height;
		blockRect.origin.y = 0;
		blockRect.origin.x += _textOffset.x;
		blockRect.intersect(Rect(0, 0, contentSize.width, contentSize.height));

		_blockNode = DrawNode::create();
		_blockNode->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
		Size textSize = Texture2D::getContentSizeWithString(_utf16Text.c_str(), fd);
		_blockNode->drawSolidRect(blockRect.origin, Vec2(blockRect.getMaxX(), blockRect.getMaxY()), Color4F(0, 0, 1, 1));

		fd._fontFillColor = Color3B(0, 255, 0);
		texture = new (std::nothrow) Texture2D;
		textureRect = blockRect;
		textureRect.origin.x -= _textOffset.x < 0 ? _textOffset.x : 0;
		texture->initWithString(_utf8Text.c_str(), fd);
		_blockTextSprite = Sprite::createWithTexture(texture, textureRect);
		_blockTextSprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
		_blockTextSprite->setPosition(blockRect.origin.x, 0);
		texture->release();

		_textSprite->addChild(_blockNode, -1, Node::INVALID_TAG);
		_textSprite->addChild(_blockTextSprite, 1, Node::INVALID_TAG);
	}

	{
		_cursorSprite = Sprite::create();
		_cursorSprite->setTextureRect(Rect(0, 0, 1, 20));
		_cursorSprite->setColor(Color3B(0, 0, 255));
		_cursorSprite->setPosition(cursorOffset + _textOffset.x, textSize.height / 2);

		auto cursorAction = RepeatForever::create(
			Blink::create(1, 1));

		_textSprite->addChild(_cursorSprite, 2, Node::INVALID_TAG);

		_cursorSprite->runAction(cursorAction);

	}

	Node::addChild(_textSprite, 0, Node::INVALID_TAG);
	_textSprite->retain();
	_textSprite->updateDisplayedColor(_displayedColor);
	_textSprite->updateDisplayedOpacity(_displayedOpacity);
	
	_contentDirty = false;

#if CC_LABEL_DEBUG_DRAW
	_debugDrawNode->clear();
	Vec2 vertices[4] =
	{
		Vec2::ZERO,
		Vec2(_contentSize.width, 0),
		Vec2(_contentSize.width, _contentSize.height),
		Vec2(0, _contentSize.height)
	};
	_debugDrawNode->drawPoly(vertices, 4, true, Color4F::WHITE);
#endif
}

void TextFieldTTF::deleteForward()
{
	size_t len = _inputText.length();
	if (!len || (_blockEnd == _blockStart && _cursorPos == _charCount))
	{
		// there is no string
		return;
	}

	// get the delete byte number
	size_t deleteLen = 1;    // default, erase 1 byte

	if (_blockEnd > _blockStart)
	{
		_cursorPos = _blockStart;
	}

	size_t pos = _calcCharPos(_inputText.c_str(), _cursorPos);

	if (_blockEnd > _blockStart)
	{
		deleteLen = _calcCharPos(_inputText.c_str(), _blockEnd) - pos;
		_cursorPos = _blockStart;
		_blockStart = 0;
		_blockEnd = 0;
	}
	else
	{
		while ((pos + deleteLen) != len &&
			0x80 == (0xC0 & _inputText.at(pos + deleteLen)))
		{
			++deleteLen;
		}
	}

	if (_delegate && _delegate->onTextFieldDeleteForward(this, _inputText.c_str() + len - deleteLen, static_cast<int>(deleteLen)))
	{
		// delegate doesn't want to delete backwards
		return;
	}

	// if all text deleted, show placeholder string
	if (len <= deleteLen)
	{
		_inputText = "";
		_charCount = 0;
		_cursorPos = 0;
		Label::setTextColor(_colorSpaceHolder);
		Label::setString(_placeHolder);
		return;
	}

	// set new input text
	std::string text = _inputText;
	text.erase(pos, deleteLen);
	_setString(text);
}

void TextFieldTTF::moveCursorBackward(bool wordbreak, bool selectText)
{
	if (_cursorPos == 0 && (selectText && _blockStart == 0 && _blockEnd != 0 || !selectText && _blockEnd == 0))
	{
		return;
	}

	if (selectText)
	{
		if (_cursorPos > _blockEnd)
		{
			_blockEnd = _cursorPos;
			_blockStart = _cursorPos - 1;
		}
		else if (_cursorPos == _blockStart)
		{
			_blockStart = _cursorPos - 1;
		}
		else
		{
			_blockEnd = _cursorPos - 1;
		}
	}
	else
	{
		_blockStart = _blockEnd = 0;
	}

	if (wordbreak)
	{
		int cursorPos = _cursorPos;
		int n = _calcCharPos(_inputText.c_str(), _cursorPos)-1;
		char ch = 0;
		char first = 0;
		bool firstSpace = false;
		while (n >= 0 && (ch = _inputText[n]))
		{
			CC_BREAK_IF(!ch);

			--n;
			if (0x80 != (0xC0 & ch))
			{
				--cursorPos;
				if (!first)
				{
					if (ch < 0)
						break;

					if (!isspace(ch))
						first = ch;
					else if (cursorPos + 1 == _cursorPos && !firstSpace)
						firstSpace = true;

					continue;
				}

				if (first > 0)
				{
					if (ch < 0 || (isalnum(first) && ispunct(ch)) ||
						(isalnum(ch) && ispunct(first)))
					{
						++cursorPos;
						break;
					}
					else if (!isspace(first) && isspace(ch))
					{
						if (firstSpace)
						{
							++cursorPos;
							break;
						}

						ch = n > 1 ? _inputText[n - 1] : 0;
						if (ch <= 0 || (ch && !isspace(ch)))
							break;
					}
				}
			}
		}
		if (cursorPos <= _charCount && _cursorPos != cursorPos)
		{
			_cursorPos = cursorPos;
		}

	}
	else
		_cursorPos--;
	

	_contentDirty = true;
}

void TextFieldTTF::moveCursorForward(bool wordbreak, bool selectText)
{
	if (_cursorPos == _charCount && (selectText && _blockEnd == _charCount || !selectText && _blockEnd == 0))
	{
		return;
	}
	
	if (selectText)
	{
		if (_cursorPos > _blockEnd)
		{
			_blockStart = _cursorPos;
			_blockEnd = _cursorPos + 1;
		}
		else if (_cursorPos == _blockEnd)
		{
			_blockEnd  = _cursorPos + 1;
		}
		else
		{
			_blockStart = _cursorPos + 1;
		}
	}
	else
	{
		_blockStart = _blockEnd = 0;
	}

	if (wordbreak)
	{
		const char* text = _inputText.c_str();
		int cursorPos = 0;
		char ch = 0;
		char first = 0;
		bool firstSpace = false;
		while ((ch = *text))
		{
			CC_BREAK_IF(!ch);

			++text;
			if (0x80 != (0xC0 & ch))
			{
				++cursorPos;
				if (cursorPos - 1 < _cursorPos)
				{
					continue;
				}
				else if (!first)
				{
					if (ch < 0)
						break;

					if (!isspace(ch))
						first = ch;
					else if (cursorPos - 1 == _cursorPos && !firstSpace)
						firstSpace = true;
					
					continue;
				}

				if (first > 0)
				{
					if (ch < 0 || (isalnum(first) && ispunct(ch)) ||
						(isalnum(ch) && ispunct(first)))
					{
						--cursorPos;
						break;
					}
					else if (!isspace(first) && isspace(ch))
					{
						if (firstSpace)
						{
							--cursorPos;
							break;
						}

						ch = *(text + 1); 
						if (ch <= 0 || (ch && !isspace(ch)))
							break;
					}
				}
			}
		}
		if (cursorPos <= _charCount && _cursorPos != cursorPos)
		{
			_cursorPos = cursorPos;
		}
		
	}
	else
		_cursorPos++;

	_contentDirty = true;
}

void TextFieldTTF::_setString(const std::string& text)
{
	static char bulletString[] = { (char)0xe2, (char)0x80, (char)0xa2, (char)0x00 };
	std::string displayText;
	size_t length;

	if (text.length() > 0)
	{
		_inputText = text;
		displayText = _inputText;
		if (_secureTextEntry)
		{
			displayText = "";
			length = _inputText.length();
			while (length)
			{
				displayText.append(bulletString);
				--length;
			}
		}
	}
	else
	{
		_inputText = "";
	}

	// if there is no input text, display placeholder instead
	if (0 == _inputText.length())
	{
		Label::setTextColor(_colorSpaceHolder);
		Label::setString(_placeHolder);
	}
	else
	{
		Label::setTextColor(_colorText);
		Label::setString(displayText);
	}
	_charCount = _calcCharCount(_inputText.c_str());
}

void TextFieldTTF::moveCursorEnd(bool selectText)
{
	if (_cursorPos == _charCount && (selectText && _blockEnd == _charCount || !selectText && _blockEnd == 0))
	{
		return;
	}

	if (selectText)
	{
		if (_cursorPos == _blockEnd)
		{
			if (_cursorPos < _charCount)
				_blockEnd = _charCount;
		}
		else
		{
			_blockStart = _blockEnd;
			_blockEnd = _charCount;
		}
	}
	else
	{
		_blockStart = _blockEnd = 0;
	}
	_cursorPos = _charCount;

	_contentDirty = true;
}

void TextFieldTTF::moveCursorHome(bool selectText)
{
	if (_cursorPos == 0 && (selectText && _blockStart == 0 && _blockEnd != 0 || !selectText && _blockEnd == 0))
	{
		return;
	}

	if (selectText)
	{
		if (_cursorPos == _blockStart)
		{
			if (_blockStart > 0)
				_blockStart = 0;
		}
		else
		{
			_blockEnd = _blockStart;
			_blockStart = 0;
		}
	}
	else
	{
		_blockStart = _blockEnd = 0;
	}
	_cursorPos = 0;

	_contentDirty = true;
}

void TextFieldTTF::handleMouseDown(Event *unusedEvent)
{
	if (_charCount < 1)
	{
		if (!_cursorSprite)
			_contentDirty = true;
		return;
	}

	EventMouse* mouseEvent = static_cast<EventMouse*>(unusedEvent);
	Vec2 pt;
	pt.x = mouseEvent->getCursorX();
	pt.y = mouseEvent->getCursorY();
	pt = convertToNodeSpace(pt);

	int cursorPos = pointToCursor(pt);

	if (cursorPos == _cursorPos && (((mouseEvent->getMods() & GLFW_MOD_SHIFT != 0) && _blockEnd != 0) || 
		((mouseEvent->getMods() & GLFW_MOD_SHIFT == 0) && _blockStart == 0 && _blockEnd == 0)))
	{
		if (!_cursorSprite)
			_contentDirty = true;
		return;
	}
	
	if (mouseEvent->getMods() & GLFW_MOD_SHIFT)
	{
		if (_blockStart == 0 && _blockEnd == 0)
		{
			_blockStart = std::min(_cursorPos, cursorPos);
			_blockEnd = std::max(_cursorPos, cursorPos);
		}
		else
		{
			if (_cursorPos == _blockStart)
			{
				if (cursorPos <= _blockEnd)
				{
					_blockStart = cursorPos;
				}
				else if (cursorPos > _blockEnd)
				{
					_blockStart = _blockEnd;
					_blockEnd = cursorPos;
				}
			}
			else if (_cursorPos == _blockEnd)
			{
				if (cursorPos >= _blockStart)
				{
					_blockEnd = cursorPos;
				}
				else if (cursorPos < _blockStart)
				{
					_blockEnd = _blockStart;
					_blockStart = cursorPos;
				}
			}
			else
			{
				_blockStart = _blockEnd = 0;
			}
		}
	}
	else
	{
		if (_blockStart != 0)
			_blockStart = 0;
		if (_blockEnd != 0)
			_blockEnd = 0;
	}

	_cursorPos = cursorPos;

	_contentDirty = true;
}

void TextFieldTTF::handleMouseMove(Event *unusedEvent)
{
	if (_charCount < 1)
		return;

	EventMouse* mouseEvent = static_cast<EventMouse*>(unusedEvent);

	if (mouseEvent->getMouseButton() == -1)
		return;

	Vec2 pt;
	pt.x = mouseEvent->getCursorX();
	pt.y = mouseEvent->getCursorY();
	pt = convertToNodeSpace(pt);

	int cursorPos = pointToCursor(pt);

	if (cursorPos == _cursorPos)
		return;

	if (_blockStart == 0 && _blockEnd == 0)
	{
		_blockStart = std::min(_cursorPos, cursorPos);
		_blockEnd = std::max(_cursorPos, cursorPos);
	}
	else
	{
		if (_cursorPos == _blockStart)
		{
			if (cursorPos <= _blockEnd)
			{
				_blockStart = cursorPos;
			}
			else if (cursorPos > _blockEnd)
			{
				_blockStart = _blockEnd;
				_blockEnd = cursorPos;
			}
		}
		else if (_cursorPos == _blockEnd)
		{
			if (cursorPos >= _blockStart)
			{
				_blockEnd = cursorPos;
			}
			else if (cursorPos < _blockStart)
			{
				_blockEnd = _blockStart;
				_blockStart = cursorPos;
			}
		}
		else
		{
			_blockStart = _blockEnd = 0;
		}
	}

	_cursorPos = cursorPos;

	_contentDirty = true;
}

void TextFieldTTF::handleMouseDblClk(Event *unusedEvent)
{
	if (_charCount < 1)
		return;

	setSelectedText(0, -1);
}

void TextFieldTTF::setSelectedText(int blockStart, int blockEnd)
{
	if (blockEnd == -1)
	{
		blockEnd = _charCount;
	}
	
	if (blockStart >= blockEnd)
	{
		return;
	}

	_blockStart = blockStart;
	_blockEnd = blockEnd;

	_contentDirty = true;
}

int TextFieldTTF::pointToCursor(Vec2& pt)
{
	if (_charCount < 1 || pt.x <= 0)
		return 0;
	
	FontDefinition fd = _getFontDefinition();
	fd._dimensions = Size::ZERO;
	fd._alignment = TextHAlignment::LEFT;
	Size textSize = Texture2D::getContentSizeWithString(_utf16Text.c_str(), fd);

	if (pt.x >= textSize.width)
		return _charCount;

	std::u16string tempText;
	int blockStart = 0;
	int blockEnd = _charCount;
	
	int blockMid = 0;
	float x1 = 0.0f, x2 = textSize.width, mid = 0.0f;
	do 
	{ 
		if (blockEnd - blockStart == 1)
		{
			if (pt.x >= x1 + (x2 - x1) / 2)
			{
				return blockEnd;
			}
			else
			{
				return blockStart;
			}
		}
		else
		{
			if (pt.x >= x1 && pt.x <= x2)
			{
				blockMid = (blockStart + blockEnd) / 2;
				tempText = _utf16Text.substr(0, blockMid);
				mid = Texture2D::getContentSizeWithString(tempText.c_str(), fd).width;
				if (pt.x >= mid)
				{
					blockStart = blockMid;
					x1 = mid;
				}
				else
				{
					blockEnd = blockMid;
					x2 = mid;
				}
			}
		}
	} while (true);
	
}

void TextFieldTTF::selectAllText()
{
	setSelectedText(0, -1);
}
std::string TextFieldTTF::getSelectedText()
{
	std::string selectText;
	if (_blockStart != _blockEnd)
	{
		std::u16string tempStr;
		tempStr = _utf16Text.substr(_blockStart, _blockEnd - _blockStart);
		StringUtils::UTF16ToUTF8(tempStr, selectText);
	}
	return selectText;
}

void TextFieldTTF::handleMouseLeave(Event *unusedEvent)
{
	if (_cursorSprite)
	{
		_cursorSprite->removeFromParent();
		_cursorSprite = nullptr;
	}
}


NS_CC_END
