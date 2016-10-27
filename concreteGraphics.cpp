#include "concreteGraphics.h"
#include <cmath>



CharacterBar::CharacterBar(const int textureID, const float width, const float height, const float step)
{
	m_textureID = textureID;

	m_width = width;
	m_height = height;
	m_step = step;

	m_sizeModifier = 0.9f;
}

void CharacterBar::setPosition(const Vector pos)
{
	m_position = pos;
}

void CharacterBar::setCurrent(const float current)
{
	m_currentValue = current;
}

void CharacterBar::setMax(const float max)
{
	m_maxValue = max;
}

void CharacterBar::draw(GraphicsHelper* pHelper)
{
	pHelper->setAlpha(1.0f);
	pHelper->setPosition(m_position.x, m_position.y);
	pHelper->setDirection(0.0f);
	pHelper->setScale(m_width, m_height);
	pHelper->setTexture(GAME_TEXTURE_BARBACKGROUND);
	pHelper->drawRectangle();

	int completeBars = static_cast<int>(m_maxValue / m_step);
	int allBars = static_cast<int>(ceill(m_maxValue / m_step));
	float barWidth = m_width / (m_maxValue / m_step);
	float currentX = m_position.x - m_width * 0.5f;

	for (int i = 0; i < allBars; i++)
	{
		float w = barWidth;
		if (i >= completeBars)
		{
			w = m_width - completeBars * barWidth;
		}
		float minValue = i * m_step;
		float maxValue = (i + 1) * m_step < m_maxValue ? (i + 1) * m_step : m_maxValue;
		float percValue = (m_currentValue - minValue) / (maxValue - minValue);
		if (percValue > 1.0f) { percValue = 1.0f; }
		if (percValue < 0.0f) { percValue = 0.0f; }

		currentX += w * 0.5f;
		pHelper->setAlpha(percValue);
		pHelper->setPosition(currentX, m_position.y);
		pHelper->setDirection(0.0f);
		pHelper->setScale(w * m_sizeModifier, m_height * m_sizeModifier);
		pHelper->setTexture(m_textureID);
		pHelper->drawRectangle();
		currentX += w * 0.5f;
	}
}



MoveGoalGraphics::MoveGoalGraphics()
: Graphics(1u)
{
	m_animationLeft = 0.0f;
	m_animationDuration = 0.5f;
	m_currentSize = 0.0f;
	m_maxSize = 0.5f;
}

MoveGoalGraphics::~MoveGoalGraphics() {}

void MoveGoalGraphics::update(const float seconds)
{
	m_currentSize = 0.0f;
	if (m_animationLeft > 0.0f)
	{
		m_animationLeft -= seconds;
		if (m_animationLeft < 0.0f)
		{
			m_animationLeft = 0.0f;
		}
		m_currentSize = m_maxSize * m_animationLeft / m_animationDuration;
	}

	Graphics::update(seconds);
}

void MoveGoalGraphics::draw(GraphicsHelper* pHelper)
{
	pHelper->setAlpha(1.0f);
	pHelper->setPosition(m_position.x, m_position.y);
	pHelper->setDirection(m_direction);
	pHelper->setScale(m_currentSize, m_currentSize);
	pHelper->setTexture(-1);
	pHelper->drawCircle();
}

void MoveGoalGraphics::reset()
{
	m_animationLeft = m_animationDuration;
}



ArenaLevel::ArenaLevel(ArenaLevel* pParent)
: Graphics(2u)
{
	m_startX = 0.0f;
	m_startY = 0.0f;
	m_endX = 0.0f;
	m_endY = 0.0f;
	if (!pParent)
	{
		m_level = 0;
	}
	else
	{
		m_level = pParent->getLevel() + 1;
	}
	m_isTaken = false;

	m_pParent = pParent;
	for (int i = 0; i < 4; i++)
	{
		m_apSublevels[i] = 0;
	}
}

ArenaLevel::~ArenaLevel()
{
	for (int i = 0; i < 4; i++)
	{
		if (m_apSublevels[i])
		{
			delete m_apSublevels[i];
		}
	}
}

void ArenaLevel::update(const float seconds)
{
	Graphics::update(seconds);
}

void ArenaLevel::draw(GraphicsHelper* pHelper)
{
	pHelper->setAlpha(1.0f);

	if (m_isTaken)
	{
		pHelper->setPosition(getHCenter(), getVCenter());
		pHelper->setDirection(0.0f);
		pHelper->setScale(getWidth(), getHeight());
		pHelper->setTexture(5);
		pHelper->drawRectangle();
	}

	for (int i = 0; i < 4; i++)
	{
		if (m_apSublevels[i])
		{
			m_apSublevels[i]->draw(pHelper);
		}
	}
}

void ArenaLevel::createSublevels(const int depth)
{
	if (depth > 0)
	{
		for (int i = 0; i < 4; i++)
		{
			m_apSublevels[i] = new ArenaLevel(this);
		}

		float aStartX[4];
		float aStartY[4];
		float aEndX[4];
		float aEndY[4];
		float widthHalf = getWidth() * 0.5f;
		float heightHalf = getHeight() * 0.5f;

		aStartX[0] = m_startX;
		aStartY[0] = m_startY;
		aEndX[0] = m_startX + widthHalf;
		aEndY[0] = m_startY + heightHalf;

		aStartX[1] = m_startX + widthHalf;
		aStartY[1] = m_startY;
		aEndX[1] = m_endX;
		aEndY[1] = m_startY + heightHalf;

		aStartX[2] = m_startX;
		aStartY[2] = m_startY + heightHalf;
		aEndX[2] = m_startX + widthHalf;
		aEndY[2] = m_endY;

		aStartX[3] = m_startX + widthHalf;
		aStartY[3] = m_startY + heightHalf;
		aEndX[3] = m_endX;
		aEndY[3] = m_endY;

		for (int i = 0; i < 4; i++)
		{
			m_apSublevels[i]->setDimensions(aStartX[i], aStartY[i], aEndX[i], aEndY[i]);
			m_apSublevels[i]->createSublevels(depth - 1);
		}
	}
}

void ArenaLevel::setDimensions(const float startX, const float startY, const float endX, const float endY)
{
	m_startX = startX;
	m_startY = startY;
	m_endX = endX;
	m_endY = endY;
}

void ArenaLevel::setTakenMark(const bool isTaken)
{
	m_isTaken = isTaken;
}

ArenaLevel* ArenaLevel::getParent() const
{
	return m_pParent;
}

ArenaLevel* ArenaLevel::getChild(const int index) const
{
	return m_apSublevels[index];
}

int ArenaLevel::getLevel() const
{
	return m_level;
}

float ArenaLevel::getWidth() const
{
	return (m_endX - m_startX);
}

float ArenaLevel::getHeight() const
{
	return (m_endY - m_startY);
}

float ArenaLevel::getHCenter() const
{
	return (0.5f * (m_startX + m_endX));
}

float ArenaLevel::getVCenter() const
{
	return (0.5f * (m_startY + m_endY));
}



CharacterGraphics::CharacterGraphics(const unsigned id, const int textureID)
: Graphics(id), m_healthbar(GAME_TEXTURE_HEALTHBAR, 1.5f, 0.25f, 10.0f), m_manabar(GAME_TEXTURE_MANABAR, 1.5f, 0.25f, 10.0f)
{
	m_handTextureID = 4;
	m_bodyTextureID = textureID;

	m_name = "";
	m_level = 0;

	m_leftHandDirection = 45.0f;
	m_rightHandDirection = -45.0f;
	m_leftHandDistance = 0.8f;
	m_rightHandDistance = 0.8f;

	m_size = 1.0f;

	m_attackDuration = 0.3f;
	m_attackAngle = 45.0f;
	m_isRightHanded = true;

	m_deathDuration = 0.5f;
	m_respawnDuration = 1.25f;

	m_healthbar.setMax(1.0f);
	m_healthbar.setCurrent(0.0f);

	m_manabar.setMax(1.0f);
	m_manabar.setCurrent(0.0f);
}

CharacterGraphics::~CharacterGraphics() {}

void CharacterGraphics::update(const float seconds)
{
	if (m_currentAnimation == CHARACTER_ANIM_IDLE)
	{
		m_leftHandPosition = getLeftHandPosition();
		m_rightHandPosition = getRightHandPosition();
		m_size = 1.0f;
	}
	else if (m_currentAnimation == CHARACTER_ANIM_ATTACK)
	{
		if (m_animationProgress > m_attackDuration)
		{
			setAnimation(CHARACTER_ANIM_IDLE);
		}
		else
		{
			if (m_isRightHanded)
			{
				float rightHandDirection = m_rightHandDirection + m_direction - (m_attackDuration - m_animationProgress) / m_attackDuration * m_attackAngle;
				m_leftHandPosition = getLeftHandPosition();
				m_rightHandPosition = Vector(rightHandDirection) * m_rightHandDistance + m_position;
			}
			else
			{
				float leftHandDirection = m_leftHandDirection + m_direction + (m_attackDuration - m_animationProgress) / m_attackDuration * m_attackAngle;
				m_leftHandPosition = Vector(leftHandDirection) * m_leftHandDistance + m_position;
				m_rightHandPosition = getRightHandPosition();
			}
		}
		m_size = 1.0f;
	}
	else if (m_currentAnimation == CHARACTER_ANIM_DEATH)
	{
		if (m_animationProgress > m_deathDuration)
		{
			setAnimation(CHARACTER_ANIM_DEAD);
		}
		else
		{
			m_size = 1.0f - m_animationProgress / m_deathDuration;
			m_leftHandPosition = getLeftHandPosition();
			m_rightHandPosition = getRightHandPosition();
		}
	}
	else if (m_currentAnimation == CHARACTER_ANIM_DEAD)
	{
		m_size = 0.0f;
	}
	else if (m_currentAnimation == CHARACTER_ANIM_RESPAWN)
	{
		if (m_animationProgress > m_respawnDuration)
		{
			m_size = 1.0f;
			setAnimation(CHARACTER_ANIM_IDLE);
		}
		else
		{
			m_size = m_animationProgress / m_respawnDuration;
			m_leftHandPosition = getLeftHandPosition();
			m_rightHandPosition = getRightHandPosition();
		}
	}

	Graphics::update(seconds);

	m_healthbar.setPosition(m_position + Vector(0.0f, 1.5f));
	m_manabar.setPosition(m_position + Vector(0.0f, 1.2f));
}

void CharacterGraphics::draw(GraphicsHelper* pHelper)
{
	// DRAW LEFT HAND
	pHelper->setAlpha(1.0f);
	pHelper->setPosition(m_leftHandPosition.x, m_leftHandPosition.y);
	pHelper->setDirection(m_direction);
	pHelper->setScale(m_size * 0.5f, m_size * 0.5f);
	pHelper->setTexture(m_handTextureID);
	pHelper->drawCircle();

	// DRAW RIGHT HAND
	pHelper->setAlpha(1.0f);
	pHelper->setPosition(m_rightHandPosition.x, m_rightHandPosition.y);
	pHelper->setDirection(m_direction);
	pHelper->setScale(m_size * 0.5f, m_size * 0.5f);
	pHelper->setTexture(m_handTextureID);
	pHelper->drawCircle();

	// DRAW BODY
	pHelper->setAlpha(1.0f);
	pHelper->setPosition(m_position.x, m_position.y);
	pHelper->setDirection(m_direction);
	pHelper->setScale(m_size * 1.0f, m_size * 1.0f);
	pHelper->setTexture(m_bodyTextureID);
	pHelper->drawCircle();

	pHelper->setAlpha(1.0f);
	pHelper->setPosition(m_position.x - 0.425f, m_position.y + 1.9f);
	pHelper->setDirection(0.0f);
	pHelper->setScale(2.35f, 0.5f);
	pHelper->setTexture(GAME_TEXTURE_BARBACKGROUND);
	pHelper->drawRectangle();

	pHelper->setAlpha(0.6f);
	pHelper->setPosition(m_position.x - 0.425f, m_position.y + 1.9f);
	pHelper->setDirection(0.0f);
	pHelper->setFontSize(0.4f);
	pHelper->drawText(m_name.c_str());

	pHelper->setAlpha(1.0f);
	pHelper->setPosition(m_position.x - 1.2f, m_position.y + 1.35f);
	pHelper->setDirection(0.0f);
	pHelper->setScale(0.8f, 0.5f);
	pHelper->setTexture(GAME_TEXTURE_BARBACKGROUND);
	pHelper->drawRectangle();

	pHelper->setAlpha(0.6f);
	pHelper->setPosition(m_position.x - 1.2f, m_position.y + 1.35f);
	pHelper->setDirection(0.0f);
	pHelper->setFontSize(0.4f);
	pHelper->drawText(std::to_string(m_level + 1).c_str());

	m_healthbar.draw(pHelper);
	m_manabar.draw(pHelper);
}

void CharacterGraphics::setName(const char* name)
{
	m_name = name;
}

void CharacterGraphics::setLevel(const int level)
{
	m_level = level;
}

void CharacterGraphics::setCurrentHP(const float hp)
{
	m_healthbar.setCurrent(hp);
}

void CharacterGraphics::setMaxHP(const float hp)
{
	m_healthbar.setMax(hp);
}

void CharacterGraphics::setCurrentMana(const float mana)
{
	m_manabar.setCurrent(mana);
}

void CharacterGraphics::setMaxMana(const float mana)
{
	m_manabar.setMax(mana);
}

Vector CharacterGraphics::getLeftHandPosition() const
{
	Vector handPosition = Vector(m_leftHandDirection + m_direction) * m_leftHandDistance + m_position;

	return handPosition;
}

Vector CharacterGraphics::getRightHandPosition() const
{
	Vector handPosition = Vector(m_rightHandDirection + m_direction) * m_rightHandDistance + m_position;

	return handPosition;
}



MeleeCharacterGraphics::MeleeCharacterGraphics(const unsigned id, const int textureID)
: CharacterGraphics(id, textureID)
{
	m_weaponTextureID = 0;
}

MeleeCharacterGraphics::~MeleeCharacterGraphics() {}

void MeleeCharacterGraphics::update(const float seconds)
{
	if (m_isRightHanded)
	{
		m_weaponPosition = m_rightHandPosition;
		m_weaponDirection = m_direction;
	}
	else
	{
		m_weaponPosition = m_leftHandPosition;
		m_weaponDirection = m_direction;
	}

	m_weaponPosition += Vector(cosf(degreesToRads(m_direction)), sinf(degreesToRads(m_direction))) * 0.5f;

	CharacterGraphics::update(seconds);
}

void MeleeCharacterGraphics::draw(GraphicsHelper* pHelper)
{
	// DRAW WEAPON
	pHelper->setAlpha(1.0f);
	pHelper->setPosition(m_weaponPosition.x, m_weaponPosition.y);
	pHelper->setDirection(m_weaponDirection);
	pHelper->setScale(m_size * 1.0f, m_size * 1.0f);
	pHelper->setTexture(m_weaponTextureID);
	pHelper->drawRectangle();

	CharacterGraphics::draw(pHelper);
}



ProjectileGraphics::ProjectileGraphics(const unsigned id)
: Graphics(id)
{}

ProjectileGraphics::~ProjectileGraphics() {}

void ProjectileGraphics::draw(GraphicsHelper* pHelper)
{
	pHelper->setAlpha(1.0f);
	pHelper->setPosition(m_position.x, m_position.y);
	pHelper->setDirection(m_direction);
	pHelper->setScale(0.2f, 0.2f);
	pHelper->setTexture(-1);
	pHelper->drawCircle();
}