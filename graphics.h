#pragma once



#include "directX.h"
#include "..\..\common\vector.h"
#include "graphicsTextureIndices.h"
#include <map>
#include <string>



class GraphicsHelper;
class Graphics;
typedef std::map<unsigned, Graphics*> GraphicsMap;
typedef std::map<unsigned, Graphics*>::iterator GraphicsIterator;
typedef std::pair<unsigned, Graphics*> GraphicsPair;



class Graphics
{
	friend class GraphicsManager;

protected:
	GraphicsManager* m_pEntityManager;

	unsigned m_ID;
	bool m_removeMark;

	Vector m_position;
	float m_direction;
	Vector m_positionPerSec;
	float m_directionPerSec;
	float m_movementDuration;
	float m_rotationDuration;

	int m_currentAnimation;
	float m_animationProgress;

public:
	Graphics(const unsigned id);
	virtual ~Graphics();

	unsigned getID() const;

	void setRemoveMark(const bool mark);

	void setPosition(const Vector& position);
	void setDirection(const float direction);
	void setPositionPerSec(const Vector& position);
	void setDirectionPerSec(const float direction);
	void setMovementDuration(const float duration);
	void setRotationDuration(const float duration);

	Vector getPosition() const;
	float getDirection() const;

	virtual void update(const float seconds);
	virtual void draw(GraphicsHelper* pHelper) = 0;

	void setAnimation(const int animationID);
};



class GraphicsManager
{
private:
	GraphicsMap m_entities;

public:
	GraphicsManager();
	~GraphicsManager();

	bool add(Graphics* pGraphics);
	bool remove(const unsigned id);
	void clear();
	bool clearMarked();

	Graphics* getGraphicsByID(const unsigned id);
	int getCount() const;

	void update(const float seconds);
	void draw(GraphicsHelper* pHelper);
};



class TextGraphicsHelper
{
private:
	float m_glyphWidth;
	float m_glyphHeight;
	float m_glyphSpacing;

	float m_x, m_y;
	float m_fontSize;

	int m_textureIDStart;

public:
	TextGraphicsHelper();

	void setPosition(const float x, const float y);
	void setFontSize(const float size);

	void drawText(const char* text, GraphicsHelper* pHelper);

private:
	int getGlyphTextureID(const char glyph) const;

	float getGlyphWidth() const;
	float getGlyphHeight() const;
	float getGlyphSpacing() const;

	float getTextWidth(const char* text) const;
	float getTextHeight(const char* text) const;
};



class GraphicsHelper
{
private:
	float m_x, m_y;
	float m_direction;
	float m_length, m_width;
	float m_alpha;

	D3DXMATRIX m_matWorld;

	LPDIRECT3DDEVICE9 m_pDev;
	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	TextureManager* m_pTextureManager;

	TextGraphicsHelper m_textHelper;

public:
	GraphicsHelper(LPDIRECT3DDEVICE9 pDevice,
		LPDIRECT3DVERTEXBUFFER9 pVB,
		TextureManager* pTextureManager);

	void setPosition(const float x, const float y);
	void setDirection(const float direction);
	void setScale(const float length, const float width);
	void setAlpha(const float alpha);
	void setTexture(const int textureID);
	void setFontSize(const float size);

	void drawCircle();
	void drawRectangle();
	void drawText(const char* text);

private:
	void invalidateMatrix();
};