#pragma once

#include "graphics.h"



#define CHARACTER_ANIM_IDLE			0
#define CHARACTER_ANIM_ATTACK		1
#define CHARACTER_ANIM_DEATH		2
#define CHARACTER_ANIM_DEAD			3
#define CHARACTER_ANIM_RESPAWN		4



class CharacterBar
{
private:
	int m_textureID;
	float m_width;
	float m_height;

	Vector m_position;

	float m_currentValue;
	float m_maxValue;
	float m_step;

	float m_sizeModifier;

public:
	CharacterBar(const int textureID, const float width, const float height, const float step);

	void setPosition(const Vector pos);

	void setCurrent(const float current);
	void setMax(const float max);

	void draw(GraphicsHelper* pHelper);
};



class MoveGoalGraphics : public Graphics
{
protected:
	float m_animationLeft;
	float m_animationDuration;
	float m_currentSize;
	float m_maxSize;

public:
	MoveGoalGraphics();
	virtual ~MoveGoalGraphics();

	virtual void update(const float seconds);
	virtual void draw(GraphicsHelper* pHelper);

	void reset();
};



class ArenaLevel : public Graphics
{
protected:
	float m_startX;
	float m_startY;
	float m_endX;
	float m_endY;
	int m_level;
	bool m_isTaken;

	ArenaLevel* m_pParent;
	ArenaLevel* m_apSublevels[4];

public:
	ArenaLevel(ArenaLevel* pParent);
	virtual ~ArenaLevel();

	virtual void update(const float seconds);
	virtual void draw(GraphicsHelper* pHelper);

	void createSublevels(const int depth);

	void setDimensions(const float startX, const float startY, const float endX, const float endY);
	void setTakenMark(const bool isTaken);

	ArenaLevel* getParent() const;
	ArenaLevel* getChild(const int index) const;

	int getLevel() const;
	float getWidth() const;
	float getHeight() const;
	float getHCenter() const;
	float getVCenter() const;
};



class CharacterGraphics : public Graphics
{
protected:
	// BARS
	CharacterBar m_healthbar;
	CharacterBar m_manabar;

	// GRAPHICS BASE
	int m_handTextureID;
	int m_bodyTextureID;

	// INFO
	std::string m_name;
	int m_level;

	// STANDARD ANIMATION
	float m_leftHandDirection;
	float m_rightHandDirection;
	float m_leftHandDistance;
	float m_rightHandDistance;

	Vector m_leftHandPosition;
	Vector m_rightHandPosition;

	float m_size;

	// ATTACK ANIMATION
	float m_attackDuration;
	float m_attackLeft;
	float m_attackAngle;
	bool m_isRightHanded;

	// DEATH ANIMATION
	float m_deathDuration;
	float m_respawnDuration;

public:
	CharacterGraphics(const unsigned id, const int textureID);
	virtual ~CharacterGraphics();

	virtual void update(const float seconds);
	virtual void draw(GraphicsHelper* pHelper);

	void setName(const char* name);
	void setLevel(const int level);

	void setCurrentHP(const float hp);
	void setMaxHP(const float hp);
	void setCurrentMana(const float mana);
	void setMaxMana(const float mana);

protected:
	Vector getLeftHandPosition() const;
	Vector getRightHandPosition() const;
};



class MeleeCharacterGraphics : public CharacterGraphics
{
protected:
	int m_weaponTextureID;

	Vector m_weaponPosition;
	float m_weaponDirection;

public:
	MeleeCharacterGraphics(const unsigned id, const int textureID);
	virtual ~MeleeCharacterGraphics();

	virtual void update(const float seconds);
	virtual void draw(GraphicsHelper* pHelper);
};



class ProjectileGraphics : public Graphics
{
public:
	ProjectileGraphics(const unsigned id);
	virtual ~ProjectileGraphics();

	virtual void draw(GraphicsHelper* pHelper);
};