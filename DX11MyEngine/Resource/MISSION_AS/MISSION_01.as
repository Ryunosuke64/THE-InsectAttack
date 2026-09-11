
uint32 groupID1 = 0;
uint32 groupID2 = 0;
uint32 groupID3 = 0;
uint32 enemyID = 0;

uint32 wave = 0;

void mission_setup()
{
    StageEnvironmentParam environmentParam;
    environmentParam.dirLightColor     = VEC3(1.0f);              // ライトカラー
    environmentParam.dirLightIntensity = 2.5f;                    // ライトの強さ
    environmentParam.fogColor          = VEC3(0.6f, 0.0f, 0.0f);  // フォグカラー
    environmentParam.fogStart          = 0.0f;                    // フォグ開始距離
    environmentParam.fogEnd            = 0.0f;                    // フォグ最大距離
    environmentParam.dofStart          = 300.0f;                  // 被写界深度開始距離
    environmentParam.dofEnd            = 1500.0f;                 // 被写界深度最大距離
    SetStageEnvironmentParam(environmentParam);

    // 八面体 
    enemyID = SpawnEnemy(
        ENEMY_TYPE::GIANT_ANT_Normal,
        VEC3(0.0f,150.0f,0.0f),
        VEC3(0.0f),
        600.0f,
        true
    );
}

void mission_main()
{
    switch (wave) {
    case 0:
    // アリ グループ１
    groupID1 = SpawnEnemyGroup(
        ENEMY_TYPE::GIANT_ANT_Normal,
        VEC3(0.0f,0.0f,0.0f),
        50.0f,
        20,
        200.0f,
        true
    );

    // アリ グループ2
    groupID2 = SpawnEnemyGroup(
        ENEMY_TYPE::GIANT_ANT_Normal,
        VEC3(-100.0f,0.0f,100.0f),
        50.0f,
        30,
        200.0f,
        true
    );
    
    // 八面体 
    enemyID = SpawnEnemy(
        ENEMY_TYPE::OCTAHEDRON,
        VEC3(0.0f,150.0f,0.0f),
        VEC3(0.0f),
        600.0f,
        true
    );
    wave++;
        break;
    case 1:
    if(IsEnemyDead(enemyID))
    {
        // アリ グループ3
        groupID3 = SpawnEnemyGroup(
            ENEMY_TYPE::GIANT_ANT_Normal,
            VEC3(0.0f,0.0f, 0.0f),
            150.0f,
            150,
            70.0f,
            true
        );
        wave++;
    }
        break;
    case 2:
    if(GetAliveEnemyCount() < 10)
    {
        SpawnEnemyGroup(
            ENEMY_TYPE::OCTAHEDRON,
            VEC3(0.0f,150.0,0.0), 
            100.0f,
            10,
            500.0f,
            true
        );
    }
        break;
    default:
        break;
    }
}
