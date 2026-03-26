#pragma once
#include "engine/NoEngine.h"

struct RailProgressBarComponent {
	No::Entity startEntity = No::nullEntity;
	No::Entity goalEntity = No::nullEntity;
	No::Entity barBaseEntity = No::nullEntity;
	No::Entity barFillEntity = No::nullEntity;
	No::Entity playerMarkerEntity = No::nullEntity;

	No::Vector2 startPosition = { 220.0f, 56.0f };
	No::Vector2 goalPosition = { 1060.0f, 56.0f };
	No::Vector2 startGoalSize = { 20.0f, 20.0f };
	float barHeight = 12.0f;
	No::Vector2 markerSize = { 26.0f, 26.0f };

	No::Color startColor = No::Color(0.2f, 0.9f, 0.4f, 1.0f);
	No::Color goalColor = No::Color(0.95f, 0.35f, 0.25f, 1.0f);
	No::Color barBaseColor = No::Color(0.15f, 0.15f, 0.2f, 0.85f);
	No::Color barFillColor = No::Color(0.95f, 0.85f, 0.2f, 0.95f);
	No::Color markerColor = No::Color(1.0f, 1.0f, 1.0f, 1.0f);

	float displayedRatio = 0.0f;
	float markerFollowSpeed = 6.0f;
	float markerPulseTime = 0.0f;
	float markerPulseSpeed = 6.0f;
	float markerPulseScaleAmplitude = 0.12f;

	int layer = 60;
	int orderBase = 0;
};

