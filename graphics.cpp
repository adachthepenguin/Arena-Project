#include "graphics.h"



Graphics::Graphics(const unsigned id)
{
	m_ID = id;

	m_removeMark = false;

	m_positionPerSec = Vector(0.0f, 0.0f);
	m_directionPerSec = 0.0f;
	m_movementDuration = 0.0f;
	m_rotationDuration = 0.0f;

	m_currentAnimation = 0;
	m_animationProgress = 0.0f;
}

Graphics::~Graphics() {}

unsigned Graphics::getID() const
{
	return m_ID;
}

void Graphics::setRemoveMark(const bool mark)
{
	m_removeMark = mark;
}

void Graphics::setPosition(const Vector& position)
{
	m_position = position;
}

void Graphics::setDirection(const float direction)
{
	m_direction = direction;
}

void Graphics::setPositionPerSec(const Vector& position)
{
	m_positionPerSec = position;
}

void Graphics::setDirectionPerSec(const float direction)
{
	m_directionPerSec = direction;
}

void Graphics::setMovementDuration(const float duration)
{
	m_movementDuration = duration;
}

void Graphics::setRotationDuration(const float duration)
{
	m_rotationDuration = duration;
}

Vector Graphics::getPosition() const
{
	return m_position;
}

float Graphics::getDirection() const
{
	return m_direction;
}

void Graphics::update(const float seconds)
{
	float movementTime = seconds < m_movementDuration ? seconds : m_movementDuration;
	float rotationTime = seconds < m_rotationDuration ? seconds : m_rotationDuration;

	Vector positionDelta = m_positionPerSec * movementTime;
	float directionDelta = m_directionPerSec * rotationTime;

	m_position += positionDelta;
	m_direction += directionDelta;
	if (m_direction < 0.0f) { m_direction += 360.0f; }
	if (m_direction >= 360.0f) { m_direction -= 360.0f; }
	m_movementDuration -= movementTime;
	m_rotationDuration -= rotationTime;

	m_animationProgress += seconds;
}

void Graphics::setAnimation(const int animationID)
{
	m_currentAnimation = animationID;
	m_animationProgress = 0.0f;
}



GraphicsManager::GraphicsManager() {}

GraphicsManager::~GraphicsManager()
{
	clear();
}

bool GraphicsManager::add(Graphics* pGraphics)
{
	// inserted ID has to be unique, otherwise it has to be rejected
	if (getGraphicsByID(pGraphics->getID()))
	{
		return false;
	}

	m_entities.insert(GraphicsPair(pGraphics->getID(), pGraphics));
	pGraphics->m_pEntityManager = this;

	return true;
}

bool GraphicsManager::remove(const unsigned id)
{
	for (GraphicsIterator i = m_entities.begin(); i != m_entities.end(); i++)
	{
		Graphics* pGraphics = i->second;
		if (pGraphics && pGraphics->getID() == id)
		{
			delete pGraphics;
			i = m_entities.erase(i);
			return true;
		}
	}

	return false;
}

void GraphicsManager::clear()
{
	for (GraphicsIterator i = m_entities.begin(); i != m_entities.end(); i++)
	{
		Graphics* pGraphics = i->second;
		if (pGraphics)
		{
			delete pGraphics;
		}
	}

	m_entities.clear();
}

bool GraphicsManager::clearMarked()
{
	for (GraphicsIterator i = m_entities.begin(); i != m_entities.end(); i++)
	{
		Graphics* pGraphics = i->second;
		if (pGraphics && pGraphics->m_removeMark)
		{
			delete pGraphics;
			m_entities.erase(i);
			return true;
		}
	}

	return false;
}

Graphics* GraphicsManager::getGraphicsByID(const unsigned id)
{
	for (GraphicsIterator i = m_entities.begin(); i != m_entities.end(); i++)
	{
		Graphics* pGraphics = i->second;
		if (pGraphics && pGraphics->getID() == id)
		{
			return pGraphics;
		}
	}

	return 0;
}

int GraphicsManager::getCount() const
{
	return m_entities.size();
}

void GraphicsManager::update(const float seconds)
{
	for (GraphicsIterator i = m_entities.begin(); i != m_entities.end(); i++)
	{
		Graphics* pGraphics = i->second;
		if (pGraphics)
		{
			pGraphics->update(seconds);
		}
	}
}

void GraphicsManager::draw(GraphicsHelper* pHelper)
{
	for (GraphicsIterator i = m_entities.begin(); i != m_entities.end(); i++)
	{
		Graphics* pGraphics = i->second;
		if (pGraphics)
		{
			pGraphics->draw(pHelper);
		}
	}
}

TextGraphicsHelper::TextGraphicsHelper()
{
	m_glyphWidth = 0.5f;
	m_glyphHeight = 0.8f;
	m_glyphSpacing = 0.05f;

	m_fontSize = 1.0f;

	m_textureIDStart = GAME_TEXTURE_CHARACTER_A;
}

void TextGraphicsHelper::setPosition(const float x, const float y)
{
	m_x = x;
	m_y = y;
}

void TextGraphicsHelper::setFontSize(const float size)
{
	m_fontSize = size;
}

void TextGraphicsHelper::drawText(const char* text, GraphicsHelper* pHelper)
{
	int length = strlen(text);

	float startX = m_x - getTextWidth(text) * 0.5f + getGlyphWidth() * 0.5f;
	float startY = m_y + getTextHeight(text) * 0.5f - getGlyphHeight() * 0.5f;
	std::string glyphs = text;
	for (int i = 0; i < length; i++)
	{
		float x = startX + i * (getGlyphWidth() + getGlyphSpacing());
		float y = startY;
		pHelper->setPosition(x, y);
		pHelper->setDirection(0.0f);
		pHelper->setScale(getGlyphWidth(), getGlyphHeight());
		pHelper->setTexture(getGlyphTextureID(glyphs[i]));
		pHelper->drawRectangle();
	}
}

int TextGraphicsHelper::getGlyphTextureID(const char glyph) const
{
	switch (glyph)
	{
		case '1':
			return m_textureIDStart + 0;
		case '2':
			return m_textureIDStart + 1;
		case '3':
			return m_textureIDStart + 2;
		case '4':
			return m_textureIDStart + 3;
		case '5':
			return m_textureIDStart + 4;
		case '6':
			return m_textureIDStart + 5;
		case '7':
			return m_textureIDStart + 6;
		case '8':
			return m_textureIDStart + 7;
		case '9':
			return m_textureIDStart + 8;
		case '0':
			return m_textureIDStart + 9;
		case 'A':
		case 'a':
			return m_textureIDStart + 10;
		case 'B':
		case 'b':
			return m_textureIDStart + 11;
		case 'C':
		case 'c':
			return m_textureIDStart + 12;
		case 'D':
		case 'd':
			return m_textureIDStart + 13;
		case 'E':
		case 'e':
			return m_textureIDStart + 14;
		case 'F':
		case 'f':
			return m_textureIDStart + 15;
		case 'G':
		case 'g':
			return m_textureIDStart + 16;
		case 'H':
		case 'h':
			return m_textureIDStart + 17;
		case 'I':
		case 'i':
			return m_textureIDStart + 18;
		case 'J':
		case 'j':
			return m_textureIDStart + 19;
		case 'K':
		case 'k':
			return m_textureIDStart + 20;
		case 'L':
		case 'l':
			return m_textureIDStart + 21;
		case 'M':
		case 'm':
			return m_textureIDStart + 22;
		case 'N':
		case 'n':
			return m_textureIDStart + 23;
		case 'O':
		case 'o':
			return m_textureIDStart + 24;
		case 'P':
		case 'p':
			return m_textureIDStart + 25;
		case 'Q':
		case 'q':
			return m_textureIDStart + 26;
		case 'R':
		case 'r':
			return m_textureIDStart + 27;
		case 'S':
		case 's':
			return m_textureIDStart + 28;
		case 'T':
		case 't':
			return m_textureIDStart + 29;
		case 'U':
		case 'u':
			return m_textureIDStart + 30;
		case 'V':
		case 'v':
			return m_textureIDStart + 31;
		case 'W':
		case 'w':
			return m_textureIDStart + 32;
		case 'X':
		case 'x':
			return m_textureIDStart + 33;
		case 'Y':
		case 'y':
			return m_textureIDStart + 34;
		case 'Z':
		case 'z':
			return m_textureIDStart + 35;
		case '*':
			return m_textureIDStart + 36;
	}

	return 4;
}

float TextGraphicsHelper::getGlyphWidth() const
{
	return m_glyphWidth * m_fontSize;
}

float TextGraphicsHelper::getGlyphHeight() const
{
	return m_glyphHeight * m_fontSize;
}

float TextGraphicsHelper::getGlyphSpacing() const
{
	return m_glyphSpacing * m_fontSize;
}

float TextGraphicsHelper::getTextWidth(const char* text) const
{
	int glyphs = strlen(text);
	int spaces = 0;
	if (glyphs > 0)
	{
		spaces = glyphs - 1;
	}
	float width = glyphs * getGlyphWidth() + spaces * getGlyphSpacing();

	return width;
}

float TextGraphicsHelper::getTextHeight(const char* text) const
{
	return getGlyphHeight();
}





GraphicsHelper::GraphicsHelper(LPDIRECT3DDEVICE9 pDevice,
	LPDIRECT3DVERTEXBUFFER9 pVB,
	TextureManager* pTextureManager)
{
	m_pDev = pDevice;
	m_pVB = pVB;
	m_pTextureManager = pTextureManager;
}

void GraphicsHelper::setPosition(const float x, const float y)
{
	m_x = x;
	m_y = y;

	invalidateMatrix();
}

void GraphicsHelper::setDirection(const float direction)
{
	m_direction = direction;

	invalidateMatrix();
}

void GraphicsHelper::setScale(const float length, const float width)
{
	m_length = length;
	m_width = width;

	invalidateMatrix();
}

void GraphicsHelper::setAlpha(const float alpha)
{
	m_alpha = alpha;
}

void GraphicsHelper::setTexture(const int textureID)
{
	m_pDev->SetTexture(0, m_pTextureManager->getTextureFromID(textureID));
}

void GraphicsHelper::setFontSize(const float size)
{
	m_textHelper.setFontSize(size);
}

void GraphicsHelper::drawCircle()
{
	m_pDev->SetTextureStageState(0, D3DTSS_CONSTANT, D3DCOLOR_ARGB((int)(m_alpha * 255), 255, 255, 255));
	m_pDev->SetStreamSource(0, m_pVB, 0, sizeof(CUSTOMVERTEX));
	m_pDev->SetFVF(CUSTOM_FVF);
	m_pDev->SetTransform(D3DTS_WORLD, &m_matWorld);
	m_pDev->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 32);
}

void GraphicsHelper::drawRectangle()
{
	m_pDev->SetTextureStageState(0, D3DTSS_CONSTANT, D3DCOLOR_ARGB((int)(m_alpha * 255), 255, 255, 255));
	m_pDev->SetStreamSource(0, m_pVB, 0, sizeof(CUSTOMVERTEX));
	m_pDev->SetFVF(CUSTOM_FVF);
	m_pDev->SetTransform(D3DTS_WORLD, &m_matWorld);
	m_pDev->DrawPrimitive(D3DPT_TRIANGLEFAN, 34, 2);
}

void GraphicsHelper::drawText(const char* text)
{
	m_pDev->SetTextureStageState(0, D3DTSS_CONSTANT, D3DCOLOR_ARGB((int)(m_alpha * 255), 255, 255, 255));
	m_textHelper.setPosition(m_x, m_y);
	m_textHelper.drawText(text, this);
}

void GraphicsHelper::invalidateMatrix()
{
	D3DXMATRIX matTranslation, matRotation, matScaling;
	D3DXMatrixTranslation(&matTranslation, m_x, m_y, 0.0f);
	D3DXMatrixRotationZ(&matRotation, m_direction * D3DX_PI / 180.0f);
	D3DXMatrixScaling(&matScaling, m_length, m_width, 1.0f);

	m_matWorld = matScaling * matRotation * matTranslation;
}