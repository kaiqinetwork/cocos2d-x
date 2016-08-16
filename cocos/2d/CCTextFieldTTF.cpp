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
#include "platform/CCFileUtils.h"
#include "base/ccUTF8.h"
#include "2d/CCSprite.h"
#include "base/CCEventMouse.h"

NS_CC_BEGIN

#define CURSOR_TIME_SHOW_HIDE 0.5f
#define CURSOR_DEFAULT_CHAR '|'

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

static std::size_t _calcCharPos(const char * text, int charCount)
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
,_passwordStyleText("\u25CF")
, _cursorEnabled(false)
, _cursorPosition(0)
, _cursorChar(CURSOR_DEFAULT_CHAR)
, _cursorShowingTime(0.0f)
, _isAttachWithIME(false)
, _selectedTextNode(nullptr)
, _selectedTextSprite(nullptr)
, _cursorSprite(nullptr)
, _selectedTextStartPos(0)
, _selectedTextEndPos(0)
, _colorSelectedTextBg(Color4B::BLUE)
, _colorSelectedText(Color4B::BLACK)
, _cursorOffset(0.0f)
{
    _colorSpaceHolder.r = _colorSpaceHolder.g = _colorSpaceHolder.b = 127;
    _colorSpaceHolder.a = 255;
}

TextFieldTTF::~TextFieldTTF()
{
	CC_SAFE_RELEASE_NULL(_cursorSprite);
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
    setDimensions(dimensions.width, dimensions.height);
    setAlignment(alignment, TextVAlignment::CENTER);

    return initWithPlaceHolder(placeholder, fontName, fontSize);
}
bool TextFieldTTF::initWithPlaceHolder(const std::string& placeholder, const std::string& fontName, float fontSize)
{
    _placeHolder = placeholder;

    do 
    {
        // If fontName is ttf file and it corrected, use TTFConfig
        if (FileUtils::getInstance()->isFileExist(fontName))
        {
            TTFConfig ttfConfig(fontName.c_str(), fontSize, GlyphCollection::DYNAMIC);
            if (setTTFConfig(ttfConfig))
            {
                break;
            }
        }

        setSystemFontName(fontName);
        setSystemFontSize(fontSize);

    } while (false);


    Label::setTextColor(_colorSpaceHolder);
    Label::setString(_placeHolder);

#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC || CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
    // On desktop default enable cursor
    if (_currentLabelType == LabelType::TTF)
    {
        setCursorEnabled(true);
    }
#endif

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
            pGlView->setIMEKeyboardState(true);
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
            glView->setIMEKeyboardState(false);
        }
    }
    return ret;
}

void TextFieldTTF::didAttachWithIME()
{
    setAttachWithIME(true);
}

void TextFieldTTF::didDetachWithIME()
{
    setAttachWithIME(false);
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
    int pos = static_cast<int>(insert.find((char)TextFormatter::NewLine));
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

        int countInsertChar = _calcCharCount(insert.c_str());
        _charCount += countInsertChar;
		
        if (_cursorEnabled)
        {
			std::string sText(_inputText);
			if (_selectedTextEndPos > _selectedTextStartPos)
			{
				std::size_t charStartPos = _calcCharPos(sText.c_str(), _selectedTextStartPos);
				std::size_t charEndPos = _calcCharPos(sText.c_str(), _selectedTextEndPos);
				sText.replace(charStartPos, charEndPos - charStartPos, insert);

				_charCount -= charEndPos - charStartPos;
				setCursorPosition(_selectedTextStartPos + countInsertChar);
				_selectedTextStartPos = _selectedTextEndPos = 0;
			}
			else
			{
				sText.insert(_cursorPosition == _charCount ? sText.size() : _calcCharPos(sText.c_str(), _cursorPosition), insert);
				setCursorPosition(_cursorPosition + countInsertChar);
			}

			setString(sText);
        }
        else
        {
            std::string sText(_inputText);
            sText.append(insert);
            setString(sText);
        }
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
	if (!len || (_cursorPosition == 0 && _selectedTextEndPos == _selectedTextStartPos))
	{
		// there is no string
		return;
	}

    // get the delete byte number
    size_t deleteLen = 1;    // default, erase 1 byte

	size_t cursorPos = _cursorPosition;
	if (_selectedTextEndPos > _selectedTextStartPos)
	{
		cursorPos = _selectedTextEndPos;
	}

	size_t pos = _calcCharPos(_inputText.c_str(), cursorPos);

	if (_selectedTextEndPos > _selectedTextStartPos)
	{
		deleteLen = pos - _calcCharPos(_inputText.c_str(), _selectedTextStartPos);
		cursorPos = _selectedTextStartPos;
	}
	else
	{
		while (0x80 == (0xC0 & _inputText.at(pos - deleteLen)))
		{
			++deleteLen;
		}
		cursorPos -= 1;
	}

	if (_delegate && _delegate->onTextFieldDeleteBackward(this, _inputText.c_str() + pos - deleteLen, static_cast<int>(deleteLen)))
	{
		// delegate doesn't want to delete backwards
		return;
	}

	_selectedTextStartPos = 0;
	_selectedTextEndPos = 0;
	setCursorPosition(cursorPos);

    // if all text deleted, show placeholder string
    if (len <= deleteLen)
    {
        _inputText = "";
        _charCount = 0;
        setCursorPosition(0);
		Label::setTextColor(_colorSpaceHolder);
		Label::setString(_placeHolder);
        return;
    }

    // set new input text
	std::string text(_inputText.c_str());
	text.erase(pos - deleteLen, deleteLen);
	_charCount = _calcCharCount(text.c_str());
	setString(text);
}

const std::string& TextFieldTTF::getContentText()
{
    return _inputText;
}

void TextFieldTTF::setCursorPosition(std::size_t cursorPosition)
{
    if (_cursorEnabled && cursorPosition <= (std::size_t)_charCount)
    {
        _cursorPosition = cursorPosition;
        _cursorShowingTime = CURSOR_TIME_SHOW_HIDE*2.0;
    }
}

void TextFieldTTF::setCursorFromPoint(const Vec2 &point, const Camera* camera)
{
    if (_cursorEnabled)
    {
        // Reset Label, no cursor
        bool oldIsAttachWithIME = _isAttachWithIME;
        _isAttachWithIME = false;
        updateCursorDisplayText();

        Rect rect;
        rect.size = getContentSize();
        if (isScreenPointInRect(point, camera, getWorldToNodeTransform(), rect, nullptr))
        {
            int latterPosition = 0;
            for (; latterPosition < _lengthOfString; ++latterPosition)
            {
                if (_lettersInfo[latterPosition].valid)
                {
                    auto sprite = getLetter(latterPosition);
                    rect.size = sprite->getContentSize();
                    if (isScreenPointInRect(point, camera, sprite->getWorldToNodeTransform(), rect, nullptr))
                    {
                        setCursorPosition(latterPosition);
                        break;
                    }
                }
            }
            if (latterPosition == _lengthOfString)
            {
                setCursorPosition(latterPosition);
            }
        }

        // Set cursor
        _isAttachWithIME = oldIsAttachWithIME;
        updateCursorDisplayText();
    }
}

void TextFieldTTF::setAttachWithIME(bool isAttachWithIME)
{
    if (isAttachWithIME != _isAttachWithIME)
    {
        _isAttachWithIME = isAttachWithIME;

        if (_isAttachWithIME)
        {
            setCursorPosition(_charCount);
        }
        updateCursorDisplayText();
		setSelectedText(0, 0);
    }
}

void TextFieldTTF::setTextColor(const Color4B &color)
{
    _colorText = color;
    if (!_inputText.empty()) 
    {
        Label::setTextColor(_colorText);
    }
}

void TextFieldTTF::visit(Renderer *renderer, const Mat4 &parentTransform, uint32_t parentFlags)
{
	if (!_visible)
	{
		return;
	}

    if (_delegate && _delegate->onVisit(this,renderer,parentTransform,parentFlags))
    {
        return;
    }

	if (_utf8Text.empty() && _cursorEnabled)
	{
		if (_systemFontDirty || _contentDirty)
		{
			updateContent();
		}
	}
	else
	{
		Label::visit(renderer,parentTransform,parentFlags);
	}
	
	if (_cursorSprite)
	{
		_cursorSprite->visit(renderer, parentTransform, parentFlags);
	}
}

void TextFieldTTF::update(float delta)
{
    if (_cursorEnabled && _isAttachWithIME)
    {
        _cursorShowingTime -= delta;
        if (_cursorShowingTime < -CURSOR_TIME_SHOW_HIDE)
        {
            _cursorShowingTime = CURSOR_TIME_SHOW_HIDE;
        }
        
        auto sprite = getCursorSprite();

        if (sprite)
        {
            if (_cursorShowingTime >= 0.0f)
            {
                sprite->setOpacity(255);
            }
            else
            {
                sprite->setOpacity(0);
            }
            sprite->setDirty(true);
        }
    }
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
    if (_inputText.empty())
    {
        Label::setTextColor(_colorSpaceHolder);
    }
}

//////////////////////////////////////////////////////////////////////////
// properties
//////////////////////////////////////////////////////////////////////////

// input text property
void TextFieldTTF::setString(const std::string &text)
{
    std::string displayText;

    int charCount = 0;
    if (!text.empty())
    {
        _inputText = text;
        displayText = _inputText;
        charCount = _calcCharCount(_inputText.c_str());
        if (_secureTextEntry)
        {
            displayText = "";
            size_t length = charCount;
            while (length)
            {
                displayText.append(_passwordStyleText);
                --length;
            }
        }
    }
    else
    {
        _inputText = "";
    }

    if (_cursorEnabled && charCount != _charCount)
    {
        _cursorPosition = charCount;
    }

    if (_cursorEnabled)
    {
        // Need for recreate all letters in Label
        Label::removeAllChildrenWithCleanup(false);
    }

    // if there is no input text, display placeholder instead
    if (_inputText.empty() && (!_cursorEnabled || !_isAttachWithIME))
    {
        Label::setTextColor(_colorSpaceHolder);
        Label::setString(_placeHolder);
    }
    else
    {
        makeStringSupportCursor(displayText);

        Label::setTextColor(_colorText);
        Label::setString(displayText);
    }
    _charCount = charCount;
}



void TextFieldTTF::appendString(const std::string& text)
{
    insertText(text.c_str(), text.length());
}

void TextFieldTTF::makeStringSupportCursor(std::string& displayText)
{
    if (_cursorEnabled && _isAttachWithIME && _currentLabelType == Label::LabelType::TTF)
    {
        if (displayText.empty())
        {
            // \b - Next char not change x position
            displayText.push_back((char)TextFormatter::NextCharNoChangeX);
            displayText.push_back(_cursorChar);
        }
        else
        {
            StringUtils::StringUTF8 stringUTF8;

            stringUTF8.replace(displayText);

            if (_cursorPosition > stringUTF8.length())
            {
                _cursorPosition = stringUTF8.length();
            }
            std::string cursorChar;
            // \b - Next char not change x position
            cursorChar.push_back((char)TextFormatter::NextCharNoChangeX);
            cursorChar.push_back(_cursorChar);
            stringUTF8.insert(_cursorPosition, cursorChar);

            displayText = stringUTF8.getAsCharSequence();
        }
    }
}

void TextFieldTTF::updateCursorDisplayText()
{
    // Update Label content
    setString(_inputText);
}

void TextFieldTTF::setCursorChar(char cursor)
{
    if (_cursorChar != cursor)
    {
        _cursorChar = cursor;
        updateCursorDisplayText();
    }
}

void TextFieldTTF::controlKey(EventKeyboard::KeyCode keyCode, int mods)
{
    if (_cursorEnabled)
    {
        switch (keyCode)
        {
        case EventKeyboard::KeyCode::KEY_HOME:
        case EventKeyboard::KeyCode::KEY_KP_HOME:
			moveCursorHome((mods & KEYBOARD_MOD_SHIFT) != 0);
            break;
        case EventKeyboard::KeyCode::KEY_END:
			moveCursorEnd((mods & KEYBOARD_MOD_SHIFT) != 0);
            break;
        case EventKeyboard::KeyCode::KEY_DELETE:
        case EventKeyboard::KeyCode::KEY_KP_DELETE:
			deleteForward();
            break;
        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
			moveCursorBackward((mods & KEYBOARD_MOD_CONTROL) != 0, (mods & KEYBOARD_MOD_SHIFT) != 0);
            break;
        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
			moveCursorForward((mods & KEYBOARD_MOD_CONTROL) != 0, (mods & KEYBOARD_MOD_SHIFT) != 0);
            break;
        case EventKeyboard::KeyCode::KEY_ESCAPE:
            detachWithIME();
            break;
        default:
            break;
        }
    }
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

void TextFieldTTF::setCursorEnabled(bool enabled)
{
    if (_cursorEnabled != enabled)
    {
        _cursorEnabled = enabled;
        if (_cursorEnabled)
        {
            _cursorPosition = _charCount;

            scheduleUpdate();
        }
        else
        {
            _cursorPosition = 0;

            unscheduleUpdate();
        }
    }
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

void TextFieldTTF::setPasswordTextStyle(const std::string &text)
{
    if (text.length() < 1)
    {
        return;
    }

    if (text != _passwordStyleText) {
        _passwordStyleText = text;
        setString(_inputText);
    }
}

std::string TextFieldTTF::getPasswordTextStyle()const
{
    return _passwordStyleText;
}

bool TextFieldTTF::isSecureTextEntry() const
{
    return _secureTextEntry;
}

void TextFieldTTF::selectAllText()
{
	setSelectedText(0, -1);
}

void TextFieldTTF::setSelectedText(int startPos, int endPos)
{
	if (endPos == -1)
	{
		endPos = _charCount;
	}

	if (startPos > endPos)
	{
		return;
	}

	_selectedTextStartPos = startPos;
	_selectedTextEndPos = endPos;

	_contentDirty = true;
}

std::string TextFieldTTF::getSelectedText()
{
	std::string selectedText;
	if (_selectedTextStartPos != _selectedTextEndPos)
	{
		std::u16string tempStr;
		tempStr = _utf16Text.substr(_selectedTextStartPos, _selectedTextEndPos - _selectedTextStartPos);
		StringUtils::UTF16ToUTF8(tempStr, selectedText);
	}
	return selectedText;
}

void TextFieldTTF::handleMouseDown(Event *unusedEvent)
{
	if (!_cursorEnabled)
	{
		return;
	}

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

	std::size_t cursorPos = getCursorFromPoint(pt);

	if (cursorPos == _cursorPosition && (((mouseEvent->getMods() & MOUSE_MOD_SHIFT != 0) && _selectedTextEndPos != 0) ||
		((mouseEvent->getMods() & MOUSE_MOD_SHIFT == 0) && _selectedTextStartPos == 0 && _selectedTextEndPos == 0)))
	{
		if (!_cursorSprite)
			_contentDirty = true;
		return;
	}

	if (mouseEvent->getMods() & MOUSE_MOD_SHIFT)
	{
		if (_selectedTextStartPos == 0 && _selectedTextEndPos == 0)
		{
			_selectedTextStartPos = std::min(_cursorPosition, cursorPos);
			_selectedTextEndPos = std::max(_cursorPosition, cursorPos);
		}
		else
		{
			if (_cursorPosition == _selectedTextStartPos)
			{
				if (cursorPos <= _selectedTextEndPos)
				{
					_selectedTextStartPos = cursorPos;
				}
				else if (cursorPos > _selectedTextEndPos)
				{
					_selectedTextStartPos = _selectedTextEndPos;
					_selectedTextEndPos = cursorPos;
				}
			}
			else if (_cursorPosition == _selectedTextEndPos)
			{
				if (cursorPos >= _selectedTextStartPos)
				{
					_selectedTextEndPos = cursorPos;
				}
				else if (cursorPos < _selectedTextStartPos)
				{
					_selectedTextEndPos = _selectedTextStartPos;
					_selectedTextStartPos = cursorPos;
				}
			}
			else
			{
				_selectedTextStartPos = _selectedTextEndPos = 0;
			}
		}
	}
	else
	{
		if (_selectedTextStartPos != 0)
			_selectedTextStartPos = 0;
		if (_selectedTextEndPos != 0)
			_selectedTextEndPos = 0;
	}

	_cursorPosition = cursorPos;

	_contentDirty = true;
}

void TextFieldTTF::handleMouseMove(Event *unusedEvent)
{
	if (!_cursorEnabled || _charCount < 1)
	{
		return;
	}

	EventMouse* mouseEvent = static_cast<EventMouse*>(unusedEvent);

	if (mouseEvent->getMouseButton() == -1)
		return;

	Vec2 pt;
	pt.x = mouseEvent->getCursorX();
	pt.y = mouseEvent->getCursorY();
	pt = convertToNodeSpace(pt);

	std::size_t cursorPos = getCursorFromPoint(pt);

	if (cursorPos == _cursorPosition)
		return;

	if (_selectedTextStartPos == 0 && _selectedTextEndPos == 0)
	{
		_selectedTextStartPos = std::min(_cursorPosition, cursorPos);
		_selectedTextEndPos = std::max(_cursorPosition, cursorPos);
	}
	else
	{
		if (_cursorPosition == _selectedTextStartPos)
		{
			if (cursorPos <= _selectedTextEndPos)
			{
				_selectedTextStartPos = cursorPos;
			}
			else if (cursorPos > _selectedTextEndPos)
			{
				_selectedTextStartPos = _selectedTextEndPos;
				_selectedTextEndPos = cursorPos;
			}
		}
		else if (_cursorPosition == _selectedTextEndPos)
		{
			if (cursorPos >= _selectedTextStartPos)
			{
				_selectedTextEndPos = cursorPos;
			}
			else if (cursorPos < _selectedTextStartPos)
			{
				_selectedTextEndPos = _selectedTextStartPos;
				_selectedTextStartPos = cursorPos;
			}
		}
		else
		{
			_selectedTextStartPos = _selectedTextEndPos = 0;
		}
	}

	_cursorPosition = cursorPos;

	_contentDirty = true;
}

void TextFieldTTF::handleMouseDblClk(Event *unusedEvent)
{
	if (!_cursorEnabled || _charCount < 1)
	{
		return;
	}
	
	EventMouse* mouseEvent = static_cast<EventMouse*>(unusedEvent);
	if (mouseEvent->getMouseButton() == MOUSE_BUTTON_LEFT)
	{
		setSelectedText(0, -1);
	}
}

void TextFieldTTF::handleMouseLeave(Event *unusedEvent)
{
	if (_cursorSprite)
	{
		_cursorSprite->removeFromParent();
		_cursorSprite = nullptr;
	}
}

std::size_t TextFieldTTF::getCursorFromPoint(const Vec2 &point)
{
	if (!_cursorEnabled || _charCount < 1 || point.x <= 0)
	{
		return 0;
	}
	
	if (_currentLabelType == Label::LabelType::STRING_TEXTURE)
	{
		FontDefinition fd = _getFontDefinition();
		fd._dimensions = Size::ZERO;
		fd._alignment = TextHAlignment::LEFT;
		Size textSize = Texture2D::getContentSizeWithString(_utf16Text.c_str(), fd);

		if (point.x >= textSize.width)
			return _charCount;

		std::u16string tempText;
		std::size_t selectedTextStartPos = 0;
		std::size_t selectedTextEndPos = _charCount;

		std::size_t selectedTextMidPos = 0;
		float x1 = 0.0f, x2 = textSize.width, mid = 0.0f;
		do
		{
			if (selectedTextEndPos - selectedTextStartPos == 1)
			{
				if (point.x >= x1 + (x2 - x1) / 2)
				{
					return selectedTextEndPos;
				}
				else
				{
					return selectedTextStartPos;
				}
			}
			else
			{
				if (point.x >= x1 && point.x <= x2)
				{
					selectedTextMidPos = (selectedTextStartPos + selectedTextEndPos) / 2;
					tempText = _utf16Text.substr(0, selectedTextMidPos);
					mid = Texture2D::getContentSizeWithString(tempText.c_str(), fd).width;
					if (point.x >= mid)
					{
						selectedTextStartPos = selectedTextMidPos;
						x1 = mid;
					}
					else
					{
						selectedTextEndPos = selectedTextMidPos;
						x2 = mid;
					}
				}
			}
		} while (true);
	}
}

void TextFieldTTF::updateContent()
{
	_selectedTextNode = nullptr;
	_selectedTextSprite = nullptr;
	CC_SAFE_RELEASE_NULL(_cursorSprite);

	Label::updateContent();
	
	if (_currentLabelType == Label::LabelType::STRING_TEXTURE)
	{
		if (_isAttachWithIME)
		{
			FontDefinition fd = _getFontDefinition();
			fd._dimensions = Size::ZERO;
			fd._alignment = TextHAlignment::LEFT;
			Size textSize = Texture2D::getContentSizeWithString("I", fd);

			_cursorSprite = Sprite::create();
			_cursorSprite->setTextureRect(Rect(0, 0, 1, textSize.height));
			_cursorSprite->setColor(Color3B(250, 250, 250));
			_cursorSprite->setPosition(_cursorOffset + _textOffset.x, textSize.height / 2);
			_cursorSprite->retain();
		}
	}
}

Sprite* TextFieldTTF::getCursorSprite()
{
	if (!_cursorEnabled)
	{
		return nullptr;
	}

	if (_currentLabelType == Label::LabelType::TTF)
	{
		// before cursor inserted '\b', need next letter
		return getLetter((int)_cursorPosition + 1);
	}
	else
	{
		return _cursorSprite;
	}
}

void TextFieldTTF::moveCursorHome(bool selectText)
{
	if (_cursorPosition == 0 && (selectText && _selectedTextStartPos == 0 && _selectedTextEndPos != 0 || !selectText && _selectedTextEndPos == 0))
	{
		return;
	}

	if (selectText)
	{
		if (_cursorPosition == _selectedTextStartPos)
		{
			if (_selectedTextStartPos > 0)
				_selectedTextStartPos = 0;
		}
		else
		{
			_selectedTextEndPos = _selectedTextStartPos;
			_selectedTextStartPos = 0;
		}
	}
	else
	{
		_selectedTextStartPos = _selectedTextEndPos = 0;
	}
	setCursorPosition(0);

	if (_currentLabelType == Label::LabelType::TTF)
	{
		updateCursorDisplayText();
	}
	else
	{
		_contentDirty = true;
	}
}

void TextFieldTTF::moveCursorEnd(bool selectText)
{
	if (_cursorPosition == _charCount && (selectText && _selectedTextEndPos == _charCount || !selectText && _selectedTextEndPos == 0))
	{
		return;
	}

	if (selectText)
	{
		if (_cursorPosition == _selectedTextEndPos)
		{
			if (_cursorPosition < _charCount)
				_selectedTextEndPos = _charCount;
		}
		else
		{
			_selectedTextStartPos = _selectedTextEndPos;
			_selectedTextEndPos = _charCount;
		}
	}
	else
	{
		_selectedTextStartPos = _selectedTextEndPos = 0;
	}
	setCursorPosition(_charCount);

	if (_currentLabelType == Label::LabelType::TTF)
	{
		updateCursorDisplayText();
	}
	else
	{
		_contentDirty = true;
	}
}

void TextFieldTTF::moveCursorForward(bool wordbreak, bool selectText)
{
	if (_cursorPosition == _charCount && (selectText && _selectedTextEndPos == _charCount || !selectText && _selectedTextEndPos == 0))
	{
		return;
	}

	if (selectText)
	{
		if (_cursorPosition > _selectedTextEndPos)
		{
			_selectedTextStartPos = _cursorPosition;
			_selectedTextEndPos = _cursorPosition + 1;
		}
		else if (_cursorPosition == _selectedTextEndPos)
		{
			_selectedTextEndPos = _cursorPosition + 1;
		}
		else
		{
			_selectedTextStartPos = _cursorPosition + 1;
		}
	}
	else
	{
		_selectedTextStartPos = _selectedTextEndPos = 0;
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
				if (cursorPos - 1 < _cursorPosition)
				{
					continue;
				}
				else if (!first)
				{
					if (ch < 0)
						break;

					if (!isspace(ch))
						first = ch;
					else if (cursorPos - 1 == _cursorPosition && !firstSpace)
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
		if (cursorPos <= _charCount && _cursorPosition != cursorPos)
		{
			setCursorPosition(cursorPos);
		}

	}
	else
		setCursorPosition(_cursorPosition + 1);

	if (_currentLabelType == Label::LabelType::TTF)
	{
		updateCursorDisplayText();
	}
	else
	{
		_contentDirty = true;
	}
}

void TextFieldTTF::moveCursorBackward(bool wordbreak, bool selectText)
{
	if (_cursorPosition == 0 && (selectText && _selectedTextStartPos == 0 && _selectedTextEndPos != 0 || !selectText && _selectedTextEndPos == 0))
	{
		return;
	}

	if (selectText)
	{
		if (_cursorPosition > _selectedTextEndPos)
		{
			_selectedTextEndPos = _cursorPosition;
			_selectedTextStartPos = _cursorPosition - 1;
		}
		else if (_cursorPosition == _selectedTextStartPos)
		{
			_selectedTextStartPos = _cursorPosition - 1;
		}
		else
		{
			_selectedTextEndPos = _cursorPosition - 1;
		}
	}
	else
	{
		_selectedTextStartPos = _selectedTextEndPos = 0;
	}

	if (wordbreak)
	{
		int cursorPos = _cursorPosition;
		int n = _calcCharPos(_inputText.c_str(), _cursorPosition) - 1;
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
					else if (cursorPos + 1 == _cursorPosition && !firstSpace)
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
		if (cursorPos <= _charCount && _cursorPosition != cursorPos)
		{
			_cursorPosition = cursorPos;
		}

	}
	else
		setCursorPosition(_cursorPosition -1);

	if (_currentLabelType == Label::LabelType::TTF)
	{
		updateCursorDisplayText();
	}
	else
	{
		_contentDirty = true;
	}
}

void TextFieldTTF::setSelectedTextColor(const Color4B& textColor)
{
	if (_colorSelectedText == textColor)
	{
		return;
	}

	_colorSelectedText = textColor;
	if (!_inputText.empty() && _currentLabelType == LabelType::STRING_TEXTURE)
	{
		_contentDirty = true;
	}
}

void TextFieldTTF::setSelectedTextBgColor(const Color4B& color)
{
	if (_colorSelectedTextBg == color)
	{
		return;
	}

	_colorSelectedTextBg = color;
	if (_selectedTextStartPos != _selectedTextEndPos && _currentLabelType == LabelType::STRING_TEXTURE)
	{
		_contentDirty = true;
	}
}

void TextFieldTTF::createSpriteForSystemFont(const FontDefinition& fontDef)
{
	_currentLabelType = LabelType::STRING_TEXTURE;

	FontDefinition fd = fontDef;
	fd._dimensions = Size::ZERO;
	fd._alignment = TextHAlignment::LEFT;
	Size textSize;
	
	if (!_utf16Text.empty())
	{
		textSize = Texture2D::getContentSizeWithString(_utf16Text.c_str(), fd);
	}
	else
	{
		textSize = Texture2D::getContentSizeWithString(" ", fd);
		textSize.width = 0.0f;
	}

	Size contentSize = textSize;
	if (_labelWidth != 0 && _labelHeight != 0)
	{
		contentSize.width = _labelWidth;
		contentSize.height = _labelHeight;
	}
	setContentSize(contentSize);

	std::u16string tempText;
	Size tempSize;

	if (_cursorPosition > 0)
	{
		tempText = _utf16Text.substr(0, _cursorPosition);
		_cursorOffset = Texture2D::getContentSizeWithString(tempText.c_str(), fd).width;
	}
	else
		_cursorOffset = 0;

	Rect textureRect;
	if (contentSize.width > textSize.width)
	{
		switch (fontDef._alignment)
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
		if (_cursorOffset > contentSize.width)
		{
			_textOffset.x = contentSize.width - _cursorOffset;
		}
		else if ((_cursorOffset + _textOffset.x) < 0)
		{
			_textOffset.x = -_cursorOffset;
		}
		else
		{
			_textOffset.x = 0.0f;
		}
		textureRect.origin.x = -_textOffset.x;
		textureRect.origin.y = 0;
		textureRect.size.width = contentSize.width;
		textureRect.size.height = textSize.height;
	}
	switch (fontDef._vertAlignment)
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

	_textSprite->retain();
	_textSprite->updateDisplayedColor(_displayedColor);
	_textSprite->updateDisplayedOpacity(_displayedOpacity);

	std::u16string selectedText16 = _utf16Text.substr(_selectedTextStartPos, _selectedTextEndPos - _selectedTextStartPos);
	if (!selectedText16.empty())
	{
		Rect selectedTextRect;

		if (_selectedTextStartPos > 0)
		{
			tempText = _utf16Text.substr(0, _selectedTextStartPos);
			tempSize = Texture2D::getContentSizeWithString(tempText.c_str(), fd);
			selectedTextRect.origin.x = tempSize.width;
		}

		tempSize = Texture2D::getContentSizeWithString(selectedText16.c_str(), fd);
		selectedTextRect.size.width = tempSize.width;
		selectedTextRect.size.height = tempSize.height;
		selectedTextRect.origin.y = 0;
		selectedTextRect.origin.x += _textOffset.x;
		selectedTextRect.intersect(Rect(0, 0, contentSize.width, contentSize.height));

		_selectedTextNode = DrawNode::create();
		_selectedTextNode->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
		Size textSize = Texture2D::getContentSizeWithString(_utf16Text.c_str(), fd);
		_selectedTextNode->drawSolidRect(selectedTextRect.origin, Vec2(selectedTextRect.getMaxX(), selectedTextRect.getMaxY()), Color4F(_colorSelectedTextBg));

		fd._fontFillColor = Color3B(_colorSelectedText);
		auto texture = new (std::nothrow) Texture2D;
		textureRect = selectedTextRect;
		textureRect.origin.x -= _textOffset.x < 0 ? _textOffset.x : 0;
		texture->initWithString(_utf8Text.c_str(), fd);
		_selectedTextSprite = Sprite::createWithTexture(texture, textureRect);
		_selectedTextSprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
		_selectedTextSprite->setPosition(selectedTextRect.origin.x, 0);
		texture->release();

		_textSprite->addChild(_selectedTextNode, -1, Node::INVALID_TAG);
		_textSprite->addChild(_selectedTextSprite, 1, Node::INVALID_TAG);
	}
}

void TextFieldTTF::deleteForward()
{
	size_t len = _inputText.length();
	if (!len || (_cursorPosition == _charCount && _selectedTextEndPos == _selectedTextStartPos))
	{
		// there is no string
		return;
	}

	// get the delete byte number
	size_t deleteLen = 1;    // default, erase 1 byte

	size_t cursorPos = _cursorPosition;
	if (_selectedTextEndPos > _selectedTextStartPos)
	{
		cursorPos = _selectedTextStartPos;
	}

	size_t pos = _calcCharPos(_inputText.c_str(), cursorPos);

	if (_selectedTextEndPos > _selectedTextStartPos)
	{
		deleteLen = _calcCharPos(_inputText.c_str(), _selectedTextEndPos) - pos;
		cursorPos = _selectedTextStartPos;
	}
	else
	{
		while ((pos + deleteLen) != len && 
			0x80 == (0xC0 & _inputText.at(pos + deleteLen)))
		{
			++deleteLen;
		}
	}

	if (_delegate && _delegate->onTextFieldDeleteBackward(this, _inputText.c_str() + pos, static_cast<int>(deleteLen)))
	{
		// delegate doesn't want to delete backwards
		return;
	}

	_selectedTextStartPos = 0;
	_selectedTextEndPos = 0;
	setCursorPosition(cursorPos);

	// if all text deleted, show placeholder string
	if (len <= deleteLen)
	{
		_inputText = "";
		_charCount = 0;
		setCursorPosition(0);
		Label::setTextColor(_colorSpaceHolder);
		Label::setString(_placeHolder);
		return;
	}

	// set new input text
	std::string text(_inputText.c_str());
	text.erase(pos, deleteLen);
	_charCount = _calcCharCount(text.c_str());
	setString(text);
}

NS_CC_END
