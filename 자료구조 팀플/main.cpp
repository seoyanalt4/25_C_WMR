#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define START 0
#define EVENT 1
#define BATTLE 2
#define NORMAL 3
#define MAX_PLAYERS 4

typedef struct CARD {

	char card[50]; // 카드 이름 생성
	int card_type; // 카드 종류 구분
	int card_num; // 카드 갯수 세기용
	CARD* before; // 전꺼
	CARD* next; // 다음꺼

}CARD;

typedef struct tileNode {
	int tile_num;
	int type;
	int owner;
	int build_level;
	bool move;
	struct tileNode* next;
	struct tileNode* before;

	CARD* event_card_stack_top; // 플레이어 카드 뽑기

	int players[MAX_PLAYERS];    // 타일 위에 있는 플레이어 번호(1~4)
	int player_count;            // 현재 몇 명이 올라와 있는지
} tileNode;

void setCard(CARD* cards) {

	cards->before = NULL;
	cards->next = NULL;
	//cards->card_num = 0;
	strcpy(cards->card, "");

}

typedef struct SkillNode {
	char name[50];
	struct SkillNode* left;
	struct SkillNode* right;
} SkillNode;

typedef struct character { //점수는 한바퀴를 돌때마다 그리고 normal 타일을 밟았을때 몇바퀴 돈지에 비례하여 상승
	int player_num; //플레이어 구분
	int point; //점수 및 재화
	tileNode* current_tile; // 타일 노드와 플레이어 간의 상호작용
	int loop; //바퀴수
	CARD* card_head; // 소지한 카드
	int card_index; // 카드 개수 출력
	int dice; // 일반 주사위 개수
	int fix_dice; // 확정 주사위 개수
	SkillNode* skillTree;// 스킬트리 노드
	int diceSkillCooldown; // 확정 주사위 얻기 스킬  쿨타임 (0이면 사용 가능)
	int build_discount;// 건설비 할인 비율(%) 예: 20이면 20 % 할인
	bool hasUpgradeDiceSkill;     // 확정 주사위 한 개 얻기
	bool hasUpgradeResourceSkill; // 건설 비용 감소
	char chosenSkillRoot[50];  // 선택한 스킬 루트 이름 ("주사위 능력 강화" or "자원 관리")
	bool skillTreeLocked;  // 한 번 선택한 스킬루트는 고정

}character;



SkillNode* createSkillTree() {//// 스킬트리 생성 함수
	SkillNode* root = (SkillNode*)malloc(sizeof(SkillNode));
	strcpy(root->name, "스킬 선택");

	root->left = (SkillNode*)malloc(sizeof(SkillNode));
	strcpy(root->left->name, "주사위 능력 강화");

	root->right = (SkillNode*)malloc(sizeof(SkillNode));
	strcpy(root->right->name, "자원 능력 강화");

	root->left->left = root->left->right = NULL;
	root->right->left = root->right->right = NULL;

	return root;


}

void chooseSkillTree(character* player) { //처음 화면에서 스킬트리 선택
	// 이미 선택한 경우 재선택 금지
	if (player->skillTreeLocked) {
		printf("[플레이어 %d] 스킬트리는 이미 선택되었습니다: %s\n", player->player_num, player->chosenSkillRoot);
		return;
	}

	int choice = 0;
	printf("\n[플레이어 %d] 스킬트리를 선택하세요,포인트 3000점 이상일시 스킬획득 !\n", player->player_num);
	printf("1. 주사위 능력 강화 (3턴마다 확정 주사위 얻기)\n");
	printf("2. 자원 관리 능력 (건설, 업그레이드 비용 20%% 감소)\n");

	while (1) {
		printf(">> 선택 (1 or 2): ");
		if (scanf("%d", &choice) == 1 && (choice == 1 || choice == 2)) {
			break;
		}
		else {
			while (getchar() != '\n');
			printf("잘못된 입력입니다. 다시 선택하세요.\n");
		}
	}

	if (choice == 1) {
		strcpy(player->chosenSkillRoot, "주사위 능력 강화");
		printf(">> [플레이어 %d] '주사위 능력 강화' 루트를 선택했습니다.\n", player->player_num);
	}
	else {
		strcpy(player->chosenSkillRoot, "자원 관리");
		printf(">> [플레이어 %d] '자원 관리' 루트를 선택했습니다.\n", player->player_num);

	}
	player->skillTreeLocked = true;  //  한 번 선택 후 잠금
	printf(">> [플레이어 %d] '%s' 루트를 선택했습니다.\n", player->player_num, player->chosenSkillRoot);
}


void applyResourceManagementSkill(character* character) {
	if (character->build_discount == 0) {
		character->build_discount = 20;  // 20% 할인 적용
		printf("[%d번 플레이어] 스킬 [건설 비용 감소]가 적용되었습니다! (20%% 할인)\n", character->player_num);
	}
}

void upgradeSkill(character* character) {
	// 확정 주사위 한 개 얻기
	if (!character->hasUpgradeDiceSkill &&
		strstr(character->chosenSkillRoot, "주사위") != NULL &&
		strstr(character->skillTree->left->name, "주사위") != NULL) {

		if (character->point >= 3000) {
			character->skillTree->left->right = (SkillNode*)malloc(sizeof(SkillNode));
			strcpy(character->skillTree->left->right->name, "확정 주사위 한 개 얻기");
			character->hasUpgradeDiceSkill = true;
			printf("[%d번 플레이어] 스킬 획득: 확정 주사위 한 개 얻기!\n", character->player_num);
		}
		else {
			printf("[%d번 플레이어] 포인트가 부족합니다! (필요: 3000, 현재: %d)\n", character->player_num, character->point);
		}
		return;
	}

	// 건설 비용 감소
	if (!character->hasUpgradeResourceSkill &&
		strstr(character->chosenSkillRoot, "자원") != NULL &&
		strstr(character->skillTree->right->name, "자원") != NULL)

		if (character->point >= 3000) {
			character->skillTree->right->right = (SkillNode*)malloc(sizeof(SkillNode));
			strcpy(character->skillTree->right->right->name, "건설 비용 감소");
			character->hasUpgradeResourceSkill = true;
			applyResourceManagementSkill(character);
			printf("[%d번 플레이어] 스킬 획득: 건설 비용 감소!\n", character->player_num);
		}
		else {
			printf("[%d번 플레이어] 포인트가 부족합니다! (필요: 3000, 현재: %d)\n", character->player_num, character->point);
		}
	return;
}





void print_board(tileNode* head, character p[5]);

void push_card_to_stack(tileNode* tile, CARD* new_card) {

	new_card->next = tile->event_card_stack_top; // 타일의 스택 맨 위 새 카드 가리키기
	new_card->before = NULL; // 스택은 before을 사용 X
	tile->event_card_stack_top = new_card;

}



CARD* pop_card_from_tile_stack(tileNode* tile) {

	if (tile == NULL || tile->event_card_stack_top == NULL) {
		// printf("Debug: 타일 %d 스택 비어있음.\n", tile ? tile->tile_num : -1); // 디버그용
		return NULL; // 스택이 비어있음
	}

	CARD* top_card = tile->event_card_stack_top; // 맨 위 카드 가져오기
	tile->event_card_stack_top = tile->event_card_stack_top->next; // 아래 카드를 맨 위로

	top_card->next = NULL; // 카드를 스택에서 떼어 놓기

	return top_card; // 뽑은거 반환

}



void move_player(int player_num, tileNode* from, tileNode* to) {
	// 1) from 타일에서 제거
	for (int i = 0; i < from->player_count; i++) {
		if (from->players[i] == player_num) {
			// 뒤에 있는 걸 당겨와 덮고
			from->players[i] = from->players[from->player_count - 1];
			from->player_count--;
			break;
		}
	}
	// 2) to 타일에 추가
	to->players[to->player_count++] = player_num;
}

void add_card_to_player_hand(character p[5], CARD* new_card, int player_num) {
	p[player_num].card_index += 1;
	new_card->before = NULL; // 새 헤드의 before는 NULL (OK)

	new_card->next = p[player_num].card_head;

	if (new_card->next != NULL) {
		new_card->next->before = new_card;
	}

	p[player_num].card_head = new_card; // 새로 추가된 카드는 머리가 됨
	printf("%d번 플레이어의 손패에 '%s' 카드를 추가했다.!\n", player_num, new_card->card);

	//character->card_head->card_num += 1;

}

int show_player_card(character p[5], int player_num) {

	CARD* current = p[player_num].card_head;
	int card_index = 1; // 몇개있는지 알려주려고


	printf("\n---------------현재 카드 소지 목록----------------\n");
	while (current != NULL) {

		// 카드 정보 출력
		printf("%d. %s (타입 : %d)\n", card_index, current->card, current->card_type);

		current = current->next;
		// 카드 번호 증가
		card_index++;

	}

	printf("--------------------------------------------------\n");

	return card_index;

}

CARD* delete_card_from_player(character p[5], CARD* card_remove, int player_num) {

	CARD* current = p[player_num].card_head;
	p[player_num].card_index -= 1;
	while (current != NULL) {

		if (current == card_remove) {

			if (current->before != NULL) {

				current->before->next = current->next; // 제거할 카드 빼놓고 연결하기

			}
			else { // 카드가 하나 남았을 떄

				p[player_num].card_head = current->next;

			}

			if (current->next != NULL) {

				current->next->before = current->before; // 

			}

			current->before = NULL;
			current->next = NULL;

			printf("%d번 플레이어의 손패에서 '%s' 카드를 제거 했습니다.\n", player_num, current->card);
			return current;

		}

		current = current->next;

	}


}

bool is_player_hand_empty(character p[5], int player_num) {

	if (p[player_num].card_head == NULL) {

		return true;

	}

	return false;

}

void free_player_card(CARD* card) {

	CARD* current = card;
	while (current != NULL) {

		CARD* next = current->next; // 지울꺼니까 다음꺼 미리 기억
		free(current);
		current = next; // 다음 카드로 이동

	}

}

CARD* find_card_by_index(CARD* head, int index) {

	if (head == NULL || index < 1) {

		return NULL; // 리스트가 비었거나 인덱스가 1보다 작으면 찾을 수 없음
	}

	CARD* current = head; // 리스트의 시작(헤드)부터 순회 시작
	int current_index = 1; // 현재 보고 있는 카드의 번호 (1부터 시작)

	// 리스트를 순회합니다. current가 NULL이 될 때까지 (리스트 끝까지)
	while (current != NULL) {
		// 3. 현재 보고 있는 카드의 번호가 찾으려는 인덱스와 같은지 확인
		if (current_index == index) {

			return current; // 해당 카드 노드의 포인터를 반환
		}

		// 찾으려는 인덱스가 아니라면, 다음 카드로 이동
		current = current->next;
		// 현재 카드 번호를 증가
		current_index++;
	}

	return NULL;

}

void Battle_w_player(tileNode* head, character p[5], int p1, int p2) {

	int dice1 = 0;
	int dice2 = 0;
	int pointpoint;

	printf("%d번 플레이어가 %d 플레이어에게 배틀을 신청했습니다!\n굴려라!!!! 운명의 다이스 롤!!!\n", p1, p2);
	do {

		dice1 = rand() % 6 + 1;
		dice2 = rand() % 6 + 1;
		printf("%d번 플레이어는 %d 가 나왔습니다!\n", p1, dice1);
		printf("%d번 플레이어는 %d 가 나왔습니다!\n", p2, dice2);

		if (dice1 == dice2) {

			printf("뭐야? 동점이야? 다시 굴려!!!!!!!\n");

		}
		system("pause");
	} while (dice1 == dice2);

	if (dice1 > dice2) {
		pointpoint = (rand() % 20) + p[p1].loop * 10 + 80;
		p[p1].point += pointpoint;
		p[p2].point -= pointpoint;
		system("cls");
		print_board(head, p);
		printf("%d번 플레이어가 승리했습니다. %d번 플레이어로부터 일정량의 포인트(%d)를 빼앗습니다!\n", p1, p2, pointpoint);


	}

	else if (dice2 > dice1) {

		pointpoint = (rand() % 20) + p[p2].loop * 10 + 80;
		p[p2].point += pointpoint;
		p[p1].point -= pointpoint;
		system("cls");
		print_board(head, p);
		printf("%d번 플레이어가 승리했습니다. %d번 플레이어로부터 일정량의 포인트(%d)를 빼앗습니다!\n", p2, p1, pointpoint);


	}

}


tileNode* set_tile() {
	tileNode* map_head = (tileNode*)malloc(sizeof(tileNode));
	memset(map_head, 0, sizeof(tileNode));
	map_head->tile_num = 0;
	map_head->type = START;
	map_head->move = false;

	tileNode* bef_node = map_head;

	for (int i = 1; i < 24; i++) {
		tileNode* temp_node = (tileNode*)malloc(sizeof(tileNode));
		memset(temp_node, 0, sizeof(tileNode));
		temp_node->tile_num = i;

		// 타일 타입 지정
		if (i == 6 || i == 12 || i == 18) {
			temp_node->type = EVENT;
			for (int k = 0; k < 10; k++) {

				CARD* card_to_add = (CARD*)malloc(sizeof(CARD));

				int numnum; // 랜덤으로 스택에 채움
				numnum = rand() % 5 + 1;
				if (numnum == 1) {strcpy(card_to_add->card, "주사위 한개 추가");}
				if (numnum == 2) {strcpy(card_to_add->card, "배틀 카드 추가");}
				if (numnum == 3) {strcpy(card_to_add->card, "랜덤 점수 얻기 카드");}
				if (numnum == 4) { strcpy(card_to_add->card, "뒤로 가기 카드"); }
				if (numnum == 5) { strcpy(card_to_add->card, "타일 재배치 카드"); }

				card_to_add->card_type = numnum;
				//card_to_add->card_num = 0;
				card_to_add->before = NULL;
				card_to_add->next = NULL;
				push_card_to_stack(temp_node, card_to_add);

			}

			temp_node->move = false;
		}
		else if (i == 3 || i == 9 || i == 15 || i == 21) {
			temp_node->type = BATTLE;
			temp_node->move = true;
		}
		else {
			temp_node->type = NORMAL;
			temp_node->move = true;
		}

		// 연결
		temp_node->before = bef_node;
		bef_node->next = temp_node;

		bef_node = temp_node;
	}

	// 마지막 노드와 map_head 연결
	bef_node->next = map_head;
	map_head->before = bef_node;

	return map_head;
}

void render_tile(tileNode* tile, char out[7][64]) {
	const char* p1 = " ";
	const char* p2 = " ";
	const char* p3 = " ";
	const char* p4 = " ";
	const char* reset = "\033[0m";
	const char* build_color = "";
	for (int i = 0; i < tile->player_count; i++) {
		if (tile->players[i] == 1) {
			p1 = "\033[0;31m1";
		}
		else if (tile->players[i] == 2) {
			p2 = "\033[0;32m2";
		}
		else if (tile->players[i] == 3) {
			p3 = "\033[0;33m3";
		}
		else if (tile->players[i] == 4) {
			p4 = "\033[0;36m4";
		}
	}
	if (tile->owner == 1) {
		build_color = "\033[0;31m";
	}
	else if (tile->owner == 2) {
		build_color = "\033[0;32m";
	}
	else if (tile->owner == 3) {
		build_color = "\033[0;33m";
	}
	else if (tile->owner == 4) {
		build_color = "\033[0;36m";
	}
	const char* type_str = NULL;
	switch (tile->type) {
	case START:  type_str = "START";build_color = "\033[1;33m"; break;
	case EVENT:  type_str = "EVENT";build_color = "\033[1;33m"; break;
	case BATTLE: type_str = "BATTLE";build_color = "\033[1;35m"; break;
	case NORMAL: type_str = "NORMAL";break;
	}

	snprintf(out[0], 64, "%s+-----------------------+%s", build_color, reset);
	snprintf(out[1], 64, "%s| %-21s |%s", build_color, type_str, reset);

	if (tile->type == NORMAL) {
		char buildings[16] = "";
		for (int i = 0; i < tile->build_level; i++)
			strcat(buildings, "[#]");
		snprintf(out[2], 64, "%s| %-21s |%s", build_color, buildings, reset);
		snprintf(out[3], 64, "%s| 건물주: %-13d |%s", build_color, tile->owner, reset);
	}
	else {
		snprintf(out[2], 64, "%s|                       |%s", build_color, reset);
		snprintf(out[3], 64, "%s|                       |%s", build_color, reset);
	}

	snprintf(out[4], 64, "%s|                   %s %s%s %s|%s", build_color, p1, p2, reset, build_color, reset);
	snprintf(out[5], 64, "%s| 번호: %-10d  %s %s%s %s|%s", build_color, tile->tile_num, p3, p4, reset, build_color, reset);
	snprintf(out[6], 64, "%s+-----------------------+%s", build_color, reset);
}

void render_player(character p, char out[7][64]) {
	const char* color = NULL;
	const char* reset = "\033[0m";
	switch (p.player_num) {
	case 1:
		color = "\033[0;31m";
		break;
	case 2:
		color = "\033[0;32m";
		break;
	case 3:
		color = "\033[0;33m";
		break;
	case 4:
		color = "\033[0;36m";
		break;
	}
	snprintf(out[0], 64, "%s#########################%s", color, reset);
	snprintf(out[1], 64, "%s#   [ %d 번 플레이어 ]   #%s", color, p.player_num, reset);
	snprintf(out[2], 64, "%s# 점수 : %-14d #%s", color, p.point, reset);
	snprintf(out[3], 64, "%s# 바퀴수 : %-12d #%s", color, p.loop, reset);
	snprintf(out[4], 64, "%s# 일반 : %-2d / 확정 : %-2d #%s", color, p.dice, p.fix_dice, reset);
	snprintf(out[5], 64, "%s# 카드   : %-12d #%s", color, p.card_index, reset);
	snprintf(out[6], 64, "%s#########################%s", color, reset);
}

void print_tile_row(tileNode* tiles[]) {
	char lines[7][7][64];  //
	for (int i = 0; i < 7; i++)
		render_tile(tiles[i], lines[i]);

	for (int row = 0; row < 7; row++) {
		for (int i = 0; i < 7; i++)
			printf("%s ", lines[i][row]);
		printf("\n");
	}
}

void print_players(character p[5]) {
	char box[5][7][64];  // [플레이어 인덱스][줄][내용]
	for (int i = 1; i <= 4; i++) {
		render_player(p[i], box[i]);
	}

	// 각 줄을 순서대로 출력 (0~6줄)
	for (int row = 0; row < 7; row++) {
		printf("                                       ");
		for (int i = 1; i <= 4; i++) {
			printf("%s ", box[i][row]);  // 플레이어별 같은 줄 이어 출력
		}
		printf("\n");
	}
}

void print_board(tileNode* head, character p[5]) {
	tileNode* tile = head;
	tileNode* tiles[24];
	for (int i = 0; i < 24; i++) {
		tiles[i] = tile;
		tile = tile->next;
	}

	print_tile_row(&tiles[0]);

	for (int i = 0; i < 5; i++) {
		char left[7][64], right[7][64];
		render_tile(tiles[23 - i], left);
		render_tile(tiles[7 + i], right);
		for (int j = 0; j < 7; j++) {
			printf("%s", left[j]);
			for (int p = 0; p < 5; p++)
				printf("                          ");
			printf(" %s\n", right[j]);
		}
	}

	tileNode* bottom[7] = { tiles[18], tiles[17], tiles[16], tiles[15], tiles[14], tiles[13], tiles[12] };
	print_tile_row(bottom);

	print_players(p);

}

int swap_tiles(tileNode* a, tileNode* b) {
	if (a == b) return 0;
	if (a->move == true && b->move == true) {

		// a와 b의 이전/다음
		tileNode* a_prev = a->before;
		tileNode* a_next = a->next;
		tileNode* b_prev = b->before;
		tileNode* b_next = b->next;

		// a와 b가 인접한 경우 따로 처리
		if (a->next == b) {
			a->next = b_next;
			a->before = b;
			b->next = a;
			b->before = a_prev;
		}
		else if (b->next == a) {
			b->next = a_next;
			b->before = a;
			a->next = b;
			a->before = b_prev;
		}
		else {
			// 일반적 교환
			a->next = b_next;
			a->before = b_prev;
			b->next = a_next;
			b->before = a_prev;
		}

		// 연결 갱신
		a->next->before = a;
		a->before->next = a;
		b->next->before = b;
		b->before->next = b;

		return 1;
	}
	return 0;
}

void Start(tileNode* head, tileNode* start, character p[5], int player_num) {//도착한 타일이 시작일 경우
	int point;
	point = (rand() % 20) + p[player_num].loop * 10 + 80;//포인트 지급계수 바퀴수 비례
	printf("시작지점입니다. 포인트 %d를 얻습니다.\n", point);
	return;
}

void Battle(tileNode* head, tileNode* battle, character p[5], int player_num) {//도착한 타일이 배틀일 경우

	printf(">> 배틀 : 맞짱뜨자!\n");
	int input_number = 0;
	int target_player = 0;
	bool is_input_valid = false;


	//상대를 제대로 고를 때까지 반복 do-while 문
	do {

		printf("누구랑 싸울까? | 본인 순서 제외 입력 1 ~ 4\n");
		if (scanf("%49d", &input_number) == 1) {}
		else { while (getchar() != '\n'); }

		is_input_valid = true;

		if (input_number < 1 || input_number > 4) {
			while (getchar() != '\n');
			printf("유령이랑 싸우겠다는거야?\n다시 골라\n");
			is_input_valid = false;

		}

		else {

			target_player = input_number;

			if (player_num == target_player) {
				while (getchar() != '\n');
				printf("1인 2역이라도 하겠다는 거야?\n다시 골라\n");
				is_input_valid = false;


			}

		}


	} while (!is_input_valid);
	//전투 로직 추가하기
	Battle_w_player(head, p, player_num, target_player);

}

void judg(tileNode* head, character p[5], int player_num);

void Event(tileNode* head, tileNode* event, character p[5], int player_num) {//도착한 타일이 이벤트일 경우
	// 이벤트는 총 5가지정도로 구성할 예정
	tileNode* temp = p[player_num].current_tile;
	tileNode* pre = p[player_num].current_tile;
	int dice = rand() % 3 + 1;
	printf("이벤트 발생!\n");
	int random = rand() % 3 + 1;

	if (dice == 1) {

		int point_point; // 랜덤포인트 발생
		point_point = rand() % 500 + 1;
		system("cls");
		print_board(head, p);
		printf("보물발견! 포인트를 %d 만큼 획득!\n", point_point);

		p[player_num].point += point_point;

	}
	else if (dice == 2) {

		int point_point;
		point_point = rand() % 300 + 1;
		system("cls");
		print_board(head, p);
		printf("소매치기를 당했다!\n");
		printf("포인트가! %d 만큼 감소해버렸어...\n", point_point);

		p[player_num].point -= point_point;

	}
	else if (dice == 3) { //무작위로 뒤로 1~3칸 이동
		for (int i = 0; i < random; i++) {
			temp = temp->before;
		}
	move_player(player_num, p[player_num].current_tile, temp);
	p[player_num].current_tile = temp;
	system("cls");
	print_board(head, p);
	printf("미끄러졌다!\n");
	printf("뒤로 %d 칸 만큼 이동하였습니다.", random);
	judg(head, p, player_num);
	}

	system("pause");
	//카드 뽑기
	CARD* draw = pop_card_from_tile_stack(pre);

	if (draw != NULL) {
		system("cls");
		print_board(head, p);

		printf("%d플레이어가 성공적으로 카드를 뽑았습니다.\n", player_num);
		add_card_to_player_hand(p, draw, player_num);

	}
	else {

		printf("꽝! 아무것도 없지롱\n");

	}
}


void Normal(tileNode* head, tileNode* normal, character p[5], int player_num) {//도착한 타일이 노멀일 경우
	int point;
	int choice;
	int cost = 250 - (250 * p[player_num].build_discount / 100);
	int upgrade_cost = 100 - (100 * p[player_num].build_discount / 100);

	point = (rand() % 20) + p[player_num].loop * 10 + 80;//포인트 지급계수 바퀴수 비례
	if (normal->build_level == 0) {//건물이 없을경우 포인트 지급 및 건설 기회
		printf("빈땅 입니다. 포인트 %d를 얻습니다.\n 점령비용 : %d\n1.점령한다 2.다음기회에 : ", point, cost);
		while (1) {
			scanf("%d", &point);
			if (point == 1) {
				if (p[player_num].point <= 250) {
					printf("포인트가 부족합니다.\n");
					return;
				}
				else {
					normal->owner = player_num;
					normal->build_level = 1;
					p[player_num].point -= 250;
					system("cls");
					print_board(head, p);
					printf("땅을 점령하였습니다. 건물 업그레이드 비용 : %d\n 1.업그레이드 2.다음기회에 : ", upgrade_cost);
					scanf("%d", &point);
					if (point == 1) {
						if (p[player_num].point <= 100) {
							printf("포인트가 부족합니다.\n");
							return;
						}
						else {
							normal->build_level += 1;
							p[player_num].point -= 100;
							system("cls");
							print_board(head, p);
							printf("건물을 업그레이드 하였습니다. 현재레벨 %d [최대 3]\n", normal->build_level);
							return;
						}
					}
					else {
						while (getchar() != '\n');
						return;
					}

				}
			}
			else if (point == 2) {
				return;
			}
			else { while (getchar() != '\n'); }
		}
		return;
	}
	else if (normal->build_level >= 1) {//건물이 세워져 있을 경우 본인 건물일시 업그레이드 가능 상대 건물일시 통행료
		if (normal->owner == player_num) { //건물주가 도착한 플레이어와 일치시
			if (normal->build_level <= 2) {//건물 레벨이 2보다 작으면
				printf("업그레이드가 가능합니다. 건물 업그레이드 비용 : %d\n 1.업그레이드 2.다음기회에 :", upgrade_cost);
				scanf("%d", &point);
				if (point == 1) {
					if (p[player_num].point <= 100) {
						printf("포인트가 부족합니다.\n");
						return;
					}
					else {
						normal->build_level += 1;
						p[player_num].point -= 100;
						system("cls");
						print_board(head, p);
						printf("건물을 업그레이드 하였습니다. 현재레벨 %d [최대 3]\n", normal->build_level);
						return;
					}
				}
				else {
					while (getchar() != '\n');return;
				}
			}
			else if (normal->build_level == 3) {//건물레벨이 최대치인 3일경우
				printf("본인소유 최대레벨 건물땅입니다. 포인트 %d를 얻습니다.\n", point);
				return;
			}
		}
		else {//상대 소유의 땅일 경우
			point = p[normal->owner].loop * 20 + normal->build_level * 50 + 100;//통행료 계수
			p[normal->owner].point += point;
			p[player_num].point -= point;
			system("cls");
			print_board(head, p);
			printf("%d 플레이어 소유의 땅에 도착했습니다. 통행료 %d 만큼 냅니다.\n", normal->owner, point);
			if (normal->build_level <= 2) {
				printf("인수기회가 있습니다. 비용 %d\n 1.인수한다 2. 다음기회에 : ", point);
				scanf("%d", &choice);
				while (1) {
					if (choice == 1) {
						if (p[player_num].point < point) {
							printf("인수비용이 부족합니다.\n");
							return;
						}
						else {
							p[normal->owner].point += point;
							p[player_num].point -= point;
							normal->owner = player_num;
							system("cls");
							print_board(head, p);
							printf("%d 플레이어 소유의 땅을 포인트 %d을 지불하고 인수했습니다.\n 1.건물 업그레이드 비용 : %d\n 2.다음기회에 : ", normal->owner, point, upgrade_cost);
							scanf("%d", &point);
							if (point == 1) {
								if (p[player_num].point <= 100) {
									printf("포인트가 부족합니다.\n");
									return;
								}
								else {
									normal->build_level += 1;
									p[player_num].point -= 100;
									system("cls");
									print_board(head, p);
									printf("건물을 업그레이드 하였습니다. 현재레벨 %d [최대 3]\n", normal->build_level);
									return;
								}
							}
							else {
								while (getchar() != '\n');
								return;
							}
						}
					}
					else if (choice == 2) {
						while (getchar() != '\n');
						return;
					}
					else { while (getchar() != '\n'); }
				}
			}
			else if (normal->build_level == 3) {
				printf("상대의 땅이 3레벨 이므로 인수가 불가능 합니다.\n");
				return;
			}
			

		}
	}
	return;
}

void judg(tileNode* head, character p[5], int player_num) {//도착한 타일이 어떤타일인지 판단
	if (p[player_num].current_tile->type == START) {
		Start(head, p[player_num].current_tile, p, player_num);
	}
	else if (p[player_num].current_tile->type == BATTLE) {
		Battle(head, p[player_num].current_tile, p, player_num);
	}
	else if (p[player_num].current_tile->type == EVENT) {
		Event(head, p[player_num].current_tile, p, player_num);
	}
	else if (p[player_num].current_tile->type == NORMAL) {
		Normal(head, p[player_num].current_tile, p, player_num);
	}
}

void play(tileNode* head, character p[5], int player_num) {
	int choice;
	tileNode* temp = p[player_num].current_tile;
	while (1) {
		system("cls");
		print_board(head, p);
		if (p[player_num].dice == 0 && p[player_num].fix_dice == 0) { return; } //주사위가 없을때
		int endcount = 0;
		for (int i = 1;i <= 4;i++) {  //모든 플레이어 주사위가 없을때
			if (p[i].dice == 0 && p[i].fix_dice == 0) {
				endcount++;
			}
		}
		if (endcount == 4) {// 엔딩함수
			return;
		}
		printf("%d 플레이어 차례입니다. 행동을 선택해 주세요.\n1. 일반주사위 2. 확정주사위 3. 카드, 4. 스킬사용 5. 스킬 업그레이드", player_num);
		scanf("%d", &choice);
		switch (choice) {
		case 1:
			if (p[player_num].dice == 0) {

				printf("일반 주사위가 없습니다. 다시 선택하세요\n");

				continue;
			}
			else {
				choice = (rand() % 6) + 1;
				for (int i = 0; i < choice; i++) {
					temp = temp->next;
					if (temp->type == START) {
						p[player_num].loop += 1;
					}
				}
				move_player(player_num, p[player_num].current_tile, temp);
				p[player_num].current_tile = temp;
				p[player_num].dice -= 1;
				system("cls");
				print_board(head, p);
				printf("%d 칸 만큼 이동하였습니다.", choice);
				judg(head, p, player_num);
				return;
			}
			break;
		case 2:
			if (p[player_num].fix_dice == 0) {

				printf("확정 주사위가 없습니다. 다시 선택하세요\n");

				continue;
			}
			else {
				while (1) {
					printf("확정주사위를 선택하였습니다. 몇칸 이동할까요? : ");
					scanf("%d", &choice);
					if (choice <= 6 && choice >= 1) {
						for (int i = 0; i < choice; i++) {
							temp = temp->next;
							if (temp->type == START) {
								p[player_num].loop += 1;
							}
						}
						move_player(player_num, p[player_num].current_tile, temp);
						p[player_num].current_tile = temp;
						p[player_num].fix_dice -= 1;
						system("cls");
						print_board(head, p);
						printf("%d 칸 만큼 이동하였습니다.", choice);
						judg(head, p, player_num);
						return;
					}
					else { while (getchar() != '\n'); }
				}
			}
			break;
		case 3:
			printf("\n--- 카드 사용 ---\n");
			if (is_player_hand_empty(p, player_num)) { // 플레이어 손패가 비어있는지 확인
				printf("%d번 플레이어는 사용할 수 있는 카드가 없습니다!\n", player_num);
				system("pause");
				continue;

			}
			else {

				// 사용할 수 있는 거 보여주기
				printf("어떤 카드를 사용하겠습니까?\n");
				show_player_card(p, player_num);

				int card_number_to_use = 0;

				if (scanf("%49d", &card_number_to_use) == 1) {}
				else { while (getchar() != '\n'); }

				CARD* card_to_use = find_card_by_index(p[player_num].card_head, card_number_to_use);
				int use_card = 1;
				if (card_to_use != NULL) {

					printf("'%s' 카드를 사용합니다!\n", card_to_use->card);

					if (card_to_use->card_type == 1) {

						printf(">> 주사위 +1의 효과 발동!\n");
						p[player_num].dice += 1;
						system("pause");
					}

					else if (card_to_use->card_type == 2) {

						printf(">> 배틀 : 맞짱뜨자!\n");
						int input_number = 0;
						int target_player = 0;
						bool is_input_valid = false;


						//상대를 제대로 고를 때까지 반복 do-while 문
						do {

							printf("누구랑 싸울까? | 본인 순서 제외 입력 1 ~ 4\n");
							if (scanf("%49d", &input_number) == 1) {}
							else { while (getchar() != '\n'); }

							is_input_valid = true;

							if (input_number < 1 || input_number > 4) {

								printf("유령이랑 싸우겠다는거야?\n다시 골라\n");
								is_input_valid = false;

							}

							else {

								target_player = input_number;

								if (player_num == target_player) {

									printf("1인 2역이라도 하겠다는 거야?\n다시 골라\n");
									is_input_valid = false;


								}

							}


						} while (!is_input_valid);
						//전투 로직 추가하기
						Battle_w_player(head, p, player_num, target_player);


						//배틀 함수 호출

					}

					else if (card_to_use->card_type == 3) {

						int point = 0;
						point = rand() % 500 + 1;

						printf(">> 점수를 무작위로 획득합니다 (최대 500)\n");
						printf("%d 만큼의 포인트를 획득하였습니다!\n", point);

						p[player_num].point += point;
						system("pause");
					}
					else if (card_to_use->card_type == 4) {
						int random = rand() % 6 + 1;
						printf("랜덤하게 뒤로 이동합니다.\n"); //무작위로 뒤로 1~6칸 이동
						for (int i = 0; i < random; i++) {
							temp = temp->before;
						}
						move_player(player_num, p[player_num].current_tile, temp);
						p[player_num].current_tile = temp;
						system("cls");
						print_board(head, p);
						printf("뒤로 %d 칸 만큼 이동하였습니다.", random);
						judg(head, p, player_num);
					}
					else if (card_to_use->card_type == 5) { 
						int i = 0;
						int select = 0;
						printf("어떤 플레이어가 서있는 땅하고 바꿀까요 (event, start 교체불가): ");
						scanf("%d", &select);
						i = swap_tiles(p[player_num].current_tile, p[select].current_tile);
						if (i == 1) { printf("교체가 완료되었습니다."); }
						else { printf("해당 타일과 교체가 불가능합니다."); }
						use_card = 0;
						system("pause");
					}

				}
				if (use_card==1){ 
					CARD* removed_card = delete_card_from_player(p, card_to_use, player_num); 
					if (removed_card != NULL) { // 성공적으로 제거를 했을 시

						free(removed_card);
						removed_card = NULL;

					}
				}

				continue;

			}

			break;


		case 4:
			printf("\n--- 스킬 사용 ---\n");

			// 자원 관리 루트 선택자는 스킬 사용 불가
			if (strstr(p[player_num].chosenSkillRoot, "자원") != NULL) {
				printf("자원 강화는 패시브 스킬입니다. 별도 사용이 필요하지 않습니다.\n");
				system("pause");
				break;
			}

			// 확정 주사위 한 개 얻기
			if (p[player_num].hasUpgradeDiceSkill) {
				if (p[player_num].diceSkillCooldown == 0) {
					printf("스킬 [확정 주사위 한 개 얻기] 사용! 확정 주사위 +1\n");
					p[player_num].fix_dice += 1;
					p[player_num].diceSkillCooldown = 3; // 쿨타임 시작
				}
				else {
					printf("스킬 [확정 주사위 한 개 얻기]는 현재 쿨타임입니다. (%d턴 남음)\n", p[player_num].diceSkillCooldown);
				}
			}
			else {
				printf("스킬 [확정 주사위 한 개 얻기]를 아직 보유하고 있지 않습니다.\n");
			}
			system("pause");
			break;

		case 5:
			printf("\n--- 스킬 업그레이드 ---\n");

			upgradeSkill(&p[player_num]);
			system("pause");
			break;
		}


	}
}

void ending(tileNode* head, character p[5]) {
	int build_count[4][5] = {};
	int score[5] = {};
	int winner = 0; //최고점 플레이어
	int max = 0; //최고점
	head = head->next;
	while (head->type != START) {
		if (head->type == NORMAL) {
			build_count[head->build_level][head->owner]++;
		}
		head = head->next;
	}
	printf("모든 플레이어의 주사위가 없으므로 게임 종료!\n");
	for (int i = 1;i <= 4;i++) {
		score[i] = p[i].point + build_count[1][i] * 300 + build_count[2][i] * 450 + build_count[3][i] * 600 + p[i].card_index * 200;
		winner = (max <= score[i]) ? i : winner;
		max = (max <= score[i]) ? score[i] : max;
		printf("%d번 플레이어 | point : %d | 건물 : 1레벨[300point] %d개, 2레벨[450point] %d개, 3레벨[600point] %d개 | 소지한 카드수[200point] : %d개\n 총 점수 : %d 점\n", i, p[i].point, build_count[1][i], build_count[2][i], build_count[3][i], p[i].card_index, score[i]);
	}
	printf("최종우승은 ");
	for (int i = 1;i <= 4;i++) {
		if (score[winner] == score[i]) {
			printf("%d번 플레이어 ", i);
		}
	}
	printf("입니다!");
	system("pause");
}


int main() {
	// 1. 타일 생성
	srand(time(NULL));
	tileNode* head = set_tile(); // 타일맵 생성
	character p[5]; //플레이어 생성 0번 index는 미사용
	for (int i = 1; i <= 4; i++) {//플레이어 4명 초기값 설정
		p[i].player_num = i; //플레이어 번호
		p[i].loop = 0; //바퀴수
		p[i].point = 1000; //초기 점수
		p[i].current_tile = head; //플레이어가 있는 타일
		head->players[head->player_count++] = i; //초기 위치
		p[i].dice = 10; //일반주사위
		p[i].fix_dice = 5; //확정주사위
		p[i].card_index = 0; //카드갯수
		p[i].card_head = NULL; //
		p[i].skillTree = createSkillTree();// 캐릭터 스킬능력치 초기화
		p[i].build_discount = 0;// 할인율 초기화
		p[i].hasUpgradeDiceSkill = false;
		p[i].hasUpgradeResourceSkill = false;
		p[i].diceSkillCooldown = 0;
		strcpy(p[i].chosenSkillRoot, "");
		p[i].skillTreeLocked = false;
		chooseSkillTree(&p[i]);

	}




	//타일 위치 바꾸기 사용 예제
	//tileNode* temp = head;
	//for (int i = 0; i < 5; i++) temp = temp->next;
	//temp->build_level = 2;
	//temp->owner = 2;
	//tileNode* temp1 = temp;
	//temp1 = temp1->before;
	//int i = swap_tiles(temp1, temp);  //4번타일과 5번타일의 교체를 의미 (리턴으로 정수를 받아서 교체가 됐는지 안됐는지 구별 가능)
	//system("cls");
	//print_board(head, p); //보드 출력
	//if (i == 1) printf("교체완료");
	//else if (i == 0) printf("교체불가");
	//타일은 스타트랑 이벤트 타일을 제외하고는 교체 가능

	while (1) { //게임 플레이
		int endcount = 0;
		for (int j = 1;j <= 4;j++) {
			if (p[j].dice == 0 && p[j].fix_dice == 0) {
				endcount++;
			}
		}
		if (endcount == 4) {
			break;
		}
		for (int i = 1; i <= 4; i++) {
			if (p[i].diceSkillCooldown > 0)
				p[i].diceSkillCooldown--;
			play(head, p, i);
			system("pause");
		}
	}

	system("cls");
	ending(head, p);
	system("pause");

	return 0;

}