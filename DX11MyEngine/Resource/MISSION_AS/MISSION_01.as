
uint32 groupID1 = 0;
uint32 groupID2 = 0;
uint32 groupID3 = 0;
uint32 enemyID = 0;

uint32 wave = 0;
//************************************************************************
// ミッションのセットアップ
//************************************************************************
void mission_setup()
{
    StageEnvironmentParam environmentParam;
    environmentParam.dirLightColor     = VEC3(1.0f);              // ライトカラー
    environmentParam.dirLightIntensity = 2.5f;                    // ライトの強さ
    environmentParam.fogColor          = VEC3(0.6f, 0.0f, 0.0f);  // フォグカラー
    // environmentParam.fogStart       = 50.0f;                   // フォグ開始距離
    // environmentParam.fogEnd         = 150.0f;                  // フォグ最大距離
    // environmentParam.dofStart       = 50.0f;                   // 被写界深度開始距離
    // environmentParam.dofEnd         = 150.0f;                  // 被写界深度最大距離
    environmentParam.fogStart          = 0.0f;                    // フォグ開始距離
    environmentParam.fogEnd            = 0.0f;                    // フォグ最大距離
    environmentParam.dofStart          = 1000.0f;                 // 被写界深度開始距離
    environmentParam.dofEnd            = 2000.0f;                 // 被写界深度最大距離
    SetStageEnvironmentParam(environmentParam);

    
    // アリ タイプA
    SpawnEnemyGroup(
        ENEMY_TYPE::GIANT_ANT_Normal,
        VEC3(0.0f,50.0,0.0), 
        20.0f,
        1,
        100.0f,
        true
    );
}

//************************************************************************
// ミッションルーティン
//************************************************************************
void mission_main()
{
    switch (wave) {
        //************************************************************************
        //
        //  第１ウェーブ
        //
        //************************************************************************
    case 0:
    // アリ 
    // ※イベントトリガー 
    enemyID = SpawnEnemy(
        ENEMY_TYPE::GIANT_ANT_Normal,
        VEC3(0.0f, 50.0f,0.0f),
        VEC3(0.0f),
        400.0f,
        false
    );

    // アリ グループ
    groupID3 = SpawnEnemyGroup(
        ENEMY_TYPE::GIANT_ANT_Normal,
        VEC3(0.0f,30.0f, 0.0f),
        30.0f,
        130,
        300.0f,
        false
    );
    wave++;
        break;
    case 1:
        //************************************************************************
        //
        //  第２ウェーブ
        //
        //************************************************************************
    if(IsEnemyDead(enemyID))
    {
        // アリ グループ
        groupID3 = SpawnEnemyGroup(
            ENEMY_TYPE::GIANT_ANT_Normal,
            VEC3(-200.0f,30.0f, 0.0f),
            30.0f,
            50,
            70.0f,
            true
        );
        // ドローン グループ
        groupID3 = SpawnEnemyGroup(
            ENEMY_TYPE::DRONE_TYPE_A,
            VEC3(-300.0f,200.0f, 400.0f),
            30.0f,
         20,
            70.0f,
            true
        );
          // ドローン グループ
        groupID3 = SpawnEnemyGroup(
            ENEMY_TYPE::DRONE_TYPE_A,
            VEC3(300.0f,200.0f, -200.0f),
            30.0f,
            40,
            70.0f,
            true
        );
        // 八面体 グループ
        groupID3 = SpawnEnemyGroup(
            ENEMY_TYPE::OCTAHEDRON,
            VEC3(300.0f,200.0f, -200.0f),
            50.0f,
            3,
            2000.0f,
            true
        );
        wave++;
    }
        break;
    case 2:
        //************************************************************************
        //
        //  第３ウェーブ
        //
        //************************************************************************
    if(GetAliveEnemyCount() < 10)
    {
        //************************************************ */
        // 前方向
        //************************************************ */
        // 八面体 グループ
        SpawnEnemyGroup(
            ENEMY_TYPE::OCTAHEDRON,
            VEC3(0.0f,400.0,500.0), 
            30.0f,
            1,
            2500.0f,
            true
        );

        // ドローン グループ 
        SpawnEnemyGroup(
            ENEMY_TYPE::DRONE_TYPE_A,
            VEC3(0.0f,400.0, 500.0),
            200.0f,
            150,
            70.0f,
            true
        );

        //************************************************ */
        // 右方向
        //************************************************ */
        // 八面体 グループ
        SpawnEnemyGroup(
            ENEMY_TYPE::OCTAHEDRON,
            VEC3(400.0f,400.0,0.0), 
            30.0f,
            1,
            2500.0f,
            true
        );

        // ドローン グループ
        SpawnEnemyGroup(
            ENEMY_TYPE::DRONE_TYPE_A,
            VEC3(400.0f,400.0,0.0),
            200.0f,
            100,
            70.0f,
            true
        );
        
        //************************************************ */
        // 左方向
        //************************************************ */
        // 八面体 グループ
        SpawnEnemyGroup(
            ENEMY_TYPE::OCTAHEDRON,
            VEC3(-500.0f,400.0,0.0), 
            30.0f,
            1,
            2500.0f,
            true
        );

        // ドローン グループ
        SpawnEnemyGroup(
            ENEMY_TYPE::DRONE_TYPE_A,
            VEC3(-500.0f,400.0,0.0),
            200.0f,
            100,
            70.0f,
            true
        );
        wave++;
    }
        break;
    default:
        break;
    }
}
