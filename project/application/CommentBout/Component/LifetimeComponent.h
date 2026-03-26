#pragma once

/// <summary>
/// 寿命コンポーネント
/// 寿命を持たせたいエンティティにこれを付与し、LifetimeSystem で remainingTime を減算していく
/// </summary>
struct LifetimeComponent {
	float remainingTime = 0.2f;
};