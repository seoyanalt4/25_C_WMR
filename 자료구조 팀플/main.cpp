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

	char card[50]; // ī�� �̸� ����
	int card_type; // ī�� ���� ����
	int card_num; // ī�� ���� �����
	CARD* before; // ����
	CARD* next; // ������

}CARD;

typedef struct tileNode {
	int tile_num;
	int type;
	int owner;
	int build_level;
	bool move;
	struct tileNode* next;
	struct tileNode* before;

	CARD* event_card_stack_top; // �÷��̾� ī�� �̱�

	int players[MAX_PLAYERS];    // Ÿ�� ���� �ִ� �÷��̾� ��ȣ(1~4)
	int player_count;            // ���� �� ���� �ö�� �ִ���
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

typedef struct character { //������ �ѹ����� �������� �׸��� normal Ÿ���� ������� ����� ������ ����Ͽ� ���
	int player_num; //�÷��̾� ����
	int point; //���� �� ��ȭ
	tileNode* current_tile; // Ÿ�� ���� �÷��̾� ���� ��ȣ�ۿ�
	int loop; //������
	CARD* card_head; // ������ ī��
	int card_index; // ī�� ���� ���
	int dice; // �Ϲ� �ֻ��� ����
	int fix_dice; // Ȯ�� �ֻ��� ����
	SkillNode* skillTree;// ��ųƮ�� ���
	int diceSkillCooldown; // Ȯ�� �ֻ��� ��� ��ų  ��Ÿ�� (0�̸� ��� ����)
	int build_discount;// �Ǽ��� ���� ����(%) ��: 20�̸� 20 % ����
	bool hasUpgradeDiceSkill;     // Ȯ�� �ֻ��� �� �� ���
	bool hasUpgradeResourceSkill; // �Ǽ� ��� ����
	char chosenSkillRoot[50];  // ������ ��ų ��Ʈ �̸� ("�ֻ��� �ɷ� ��ȭ" or "�ڿ� ����")
	bool skillTreeLocked;  // �� �� ������ ��ų��Ʈ�� ����

}character;



SkillNode* createSkillTree() {//// ��ųƮ�� ���� �Լ�
	SkillNode* root = (SkillNode*)malloc(sizeof(SkillNode));
	strcpy(root->name, "��ų ����");

	root->left = (SkillNode*)malloc(sizeof(SkillNode));
	strcpy(root->left->name, "�ֻ��� �ɷ� ��ȭ");

	root->right = (SkillNode*)malloc(sizeof(SkillNode));
	strcpy(root->right->name, "�ڿ� �ɷ� ��ȭ");

	root->left->left = root->left->right = NULL;
	root->right->left = root->right->right = NULL;

	return root;


}

void chooseSkillTree(character* player) { //ó�� ȭ�鿡�� ��ųƮ�� ����
	// �̹� ������ ��� �缱�� ����
	if (player->skillTreeLocked) {
		printf("[�÷��̾� %d] ��ųƮ���� �̹� ���õǾ����ϴ�: %s\n", player->player_num, player->chosenSkillRoot);
		return;
	}

	int choice = 0;
	printf("\n[�÷��̾� %d] ��ųƮ���� �����ϼ���,����Ʈ 3000�� �̻��Ͻ� ��ųȹ�� !\n", player->player_num);
	printf("1. �ֻ��� �ɷ� ��ȭ (3�ϸ��� Ȯ�� �ֻ��� ���)\n");
	printf("2. �ڿ� ���� �ɷ� (�Ǽ�, ���׷��̵� ��� 20%% ����)\n");

	while (1) {
		printf(">> ���� (1 or 2): ");
		if (scanf("%d", &choice) == 1 && (choice == 1 || choice == 2)) {
			break;
		}
		else {
			while (getchar() != '\n');
			printf("�߸��� �Է��Դϴ�. �ٽ� �����ϼ���.\n");
		}
	}

	if (choice == 1) {
		strcpy(player->chosenSkillRoot, "�ֻ��� �ɷ� ��ȭ");
		printf(">> [�÷��̾� %d] '�ֻ��� �ɷ� ��ȭ' ��Ʈ�� �����߽��ϴ�.\n", player->player_num);
	}
	else {
		strcpy(player->chosenSkillRoot, "�ڿ� ����");
		printf(">> [�÷��̾� %d] '�ڿ� ����' ��Ʈ�� �����߽��ϴ�.\n", player->player_num);

	}
	player->skillTreeLocked = true;  //  �� �� ���� �� ���
	printf(">> [�÷��̾� %d] '%s' ��Ʈ�� �����߽��ϴ�.\n", player->player_num, player->chosenSkillRoot);
}


void applyResourceManagementSkill(character* character) {
	if (character->build_discount == 0) {
		character->build_discount = 20;  // 20% ���� ����
		printf("[%d�� �÷��̾�] ��ų [�Ǽ� ��� ����]�� ����Ǿ����ϴ�! (20%% ����)\n", character->player_num);
	}
}

void upgradeSkill(character* character) {
	// Ȯ�� �ֻ��� �� �� ���
	if (!character->hasUpgradeDiceSkill &&
		strstr(character->chosenSkillRoot, "�ֻ���") != NULL &&
		strstr(character->skillTree->left->name, "�ֻ���") != NULL) {

		if (character->point >= 3000) {
			character->skillTree->left->right = (SkillNode*)malloc(sizeof(SkillNode));
			strcpy(character->skillTree->left->right->name, "Ȯ�� �ֻ��� �� �� ���");
			character->hasUpgradeDiceSkill = true;
			printf("[%d�� �÷��̾�] ��ų ȹ��: Ȯ�� �ֻ��� �� �� ���!\n", character->player_num);
		}
		else {
			printf("[%d�� �÷��̾�] ����Ʈ�� �����մϴ�! (�ʿ�: 3000, ����: %d)\n", character->player_num, character->point);
		}
		return;
	}

	// �Ǽ� ��� ����
	if (!character->hasUpgradeResourceSkill &&
		strstr(character->chosenSkillRoot, "�ڿ�") != NULL &&
		strstr(character->skillTree->right->name, "�ڿ�") != NULL)

		if (character->point >= 3000) {
			character->skillTree->right->right = (SkillNode*)malloc(sizeof(SkillNode));
			strcpy(character->skillTree->right->right->name, "�Ǽ� ��� ����");
			character->hasUpgradeResourceSkill = true;
			applyResourceManagementSkill(character);
			printf("[%d�� �÷��̾�] ��ų ȹ��: �Ǽ� ��� ����!\n", character->player_num);
		}
		else {
			printf("[%d�� �÷��̾�] ����Ʈ�� �����մϴ�! (�ʿ�: 3000, ����: %d)\n", character->player_num, character->point);
		}
	return;
}





void print_board(tileNode* head, character p[5]);

void push_card_to_stack(tileNode* tile, CARD* new_card) {

	new_card->next = tile->event_card_stack_top; // Ÿ���� ���� �� �� �� ī�� ����Ű��
	new_card->before = NULL; // ������ before�� ��� X
	tile->event_card_stack_top = new_card;

}



CARD* pop_card_from_tile_stack(tileNode* tile) {

	if (tile == NULL || tile->event_card_stack_top == NULL) {
		// printf("Debug: Ÿ�� %d ���� �������.\n", tile ? tile->tile_num : -1); // ����׿�
		return NULL; // ������ �������
	}

	CARD* top_card = tile->event_card_stack_top; // �� �� ī�� ��������
	tile->event_card_stack_top = tile->event_card_stack_top->next; // �Ʒ� ī�带 �� ����

	top_card->next = NULL; // ī�带 ���ÿ��� ���� ����

	return top_card; // ������ ��ȯ

}



void move_player(int player_num, tileNode* from, tileNode* to) {
	// 1) from Ÿ�Ͽ��� ����
	for (int i = 0; i < from->player_count; i++) {
		if (from->players[i] == player_num) {
			// �ڿ� �ִ� �� ��ܿ� ����
			from->players[i] = from->players[from->player_count - 1];
			from->player_count--;
			break;
		}
	}
	// 2) to Ÿ�Ͽ� �߰�
	to->players[to->player_count++] = player_num;
}

void add_card_to_player_hand(character p[5], CARD* new_card, int player_num) {
	p[player_num].card_index += 1;
	new_card->before = NULL; // �� ����� before�� NULL (OK)

	new_card->next = p[player_num].card_head;

	if (new_card->next != NULL) {
		new_card->next->before = new_card;
	}

	p[player_num].card_head = new_card; // ���� �߰��� ī��� �Ӹ��� ��
	printf("%d�� �÷��̾��� ���п� '%s' ī�带 �߰��ߴ�.!\n", player_num, new_card->card);

	//character->card_head->card_num += 1;

}

int show_player_card(character p[5], int player_num) {

	CARD* current = p[player_num].card_head;
	int card_index = 1; // ��ִ��� �˷��ַ���


	printf("\n---------------���� ī�� ���� ���----------------\n");
	while (current != NULL) {

		// ī�� ���� ���
		printf("%d. %s (Ÿ�� : %d)\n", card_index, current->card, current->card_type);

		current = current->next;
		// ī�� ��ȣ ����
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

				current->before->next = current->next; // ������ ī�� ������ �����ϱ�

			}
			else { // ī�尡 �ϳ� ������ ��

				p[player_num].card_head = current->next;

			}

			if (current->next != NULL) {

				current->next->before = current->before; // 

			}

			current->before = NULL;
			current->next = NULL;

			printf("%d�� �÷��̾��� ���п��� '%s' ī�带 ���� �߽��ϴ�.\n", player_num, current->card);
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

		CARD* next = current->next; // ���ﲨ�ϱ� ������ �̸� ���
		free(current);
		current = next; // ���� ī��� �̵�

	}

}

CARD* find_card_by_index(CARD* head, int index) {

	if (head == NULL || index < 1) {

		return NULL; // ����Ʈ�� ����ų� �ε����� 1���� ������ ã�� �� ����
	}

	CARD* current = head; // ����Ʈ�� ����(���)���� ��ȸ ����
	int current_index = 1; // ���� ���� �ִ� ī���� ��ȣ (1���� ����)

	// ����Ʈ�� ��ȸ�մϴ�. current�� NULL�� �� ������ (����Ʈ ������)
	while (current != NULL) {
		// 3. ���� ���� �ִ� ī���� ��ȣ�� ã������ �ε����� ������ Ȯ��
		if (current_index == index) {

			return current; // �ش� ī�� ����� �����͸� ��ȯ
		}

		// ã������ �ε����� �ƴ϶��, ���� ī��� �̵�
		current = current->next;
		// ���� ī�� ��ȣ�� ����
		current_index++;
	}

	return NULL;

}

void Battle_w_player(tileNode* head, character p[5], int p1, int p2) {

	int dice1 = 0;
	int dice2 = 0;
	int pointpoint;

	printf("%d�� �÷��̾ %d �÷��̾�� ��Ʋ�� ��û�߽��ϴ�!\n������!!!! ����� ���̽� ��!!!\n", p1, p2);
	do {

		dice1 = rand() % 6 + 1;
		dice2 = rand() % 6 + 1;
		printf("%d�� �÷��̾�� %d �� ���Խ��ϴ�!\n", p1, dice1);
		printf("%d�� �÷��̾�� %d �� ���Խ��ϴ�!\n", p2, dice2);

		if (dice1 == dice2) {

			printf("����? �����̾�? �ٽ� ����!!!!!!!\n");

		}
		system("pause");
	} while (dice1 == dice2);

	if (dice1 > dice2) {
		pointpoint = (rand() % 20) + p[p1].loop * 10 + 80;
		p[p1].point += pointpoint;
		p[p2].point -= pointpoint;
		system("cls");
		print_board(head, p);
		printf("%d�� �÷��̾ �¸��߽��ϴ�. %d�� �÷��̾�κ��� �������� ����Ʈ(%d)�� ���ѽ��ϴ�!\n", p1, p2, pointpoint);


	}

	else if (dice2 > dice1) {

		pointpoint = (rand() % 20) + p[p2].loop * 10 + 80;
		p[p2].point += pointpoint;
		p[p1].point -= pointpoint;
		system("cls");
		print_board(head, p);
		printf("%d�� �÷��̾ �¸��߽��ϴ�. %d�� �÷��̾�κ��� �������� ����Ʈ(%d)�� ���ѽ��ϴ�!\n", p2, p1, pointpoint);


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

		// Ÿ�� Ÿ�� ����
		if (i == 6 || i == 12 || i == 18) {
			temp_node->type = EVENT;
			for (int k = 0; k < 10; k++) {

				CARD* card_to_add = (CARD*)malloc(sizeof(CARD));

				int numnum; // �������� ���ÿ� ä��
				numnum = rand() % 5 + 1;
				if (numnum == 1) {strcpy(card_to_add->card, "�ֻ��� �Ѱ� �߰�");}
				if (numnum == 2) {strcpy(card_to_add->card, "��Ʋ ī�� �߰�");}
				if (numnum == 3) {strcpy(card_to_add->card, "���� ���� ��� ī��");}
				if (numnum == 4) { strcpy(card_to_add->card, "�ڷ� ���� ī��"); }
				if (numnum == 5) { strcpy(card_to_add->card, "Ÿ�� ���ġ ī��"); }

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

		// ����
		temp_node->before = bef_node;
		bef_node->next = temp_node;

		bef_node = temp_node;
	}

	// ������ ���� map_head ����
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
		snprintf(out[3], 64, "%s| �ǹ���: %-13d |%s", build_color, tile->owner, reset);
	}
	else {
		snprintf(out[2], 64, "%s|                       |%s", build_color, reset);
		snprintf(out[3], 64, "%s|                       |%s", build_color, reset);
	}

	snprintf(out[4], 64, "%s|                   %s %s%s %s|%s", build_color, p1, p2, reset, build_color, reset);
	snprintf(out[5], 64, "%s| ��ȣ: %-10d  %s %s%s %s|%s", build_color, tile->tile_num, p3, p4, reset, build_color, reset);
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
	snprintf(out[1], 64, "%s#   [ %d �� �÷��̾� ]   #%s", color, p.player_num, reset);
	snprintf(out[2], 64, "%s# ���� : %-14d #%s", color, p.point, reset);
	snprintf(out[3], 64, "%s# ������ : %-12d #%s", color, p.loop, reset);
	snprintf(out[4], 64, "%s# �Ϲ� : %-2d / Ȯ�� : %-2d #%s", color, p.dice, p.fix_dice, reset);
	snprintf(out[5], 64, "%s# ī��   : %-12d #%s", color, p.card_index, reset);
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
	char box[5][7][64];  // [�÷��̾� �ε���][��][����]
	for (int i = 1; i <= 4; i++) {
		render_player(p[i], box[i]);
	}

	// �� ���� ������� ��� (0~6��)
	for (int row = 0; row < 7; row++) {
		printf("                                       ");
		for (int i = 1; i <= 4; i++) {
			printf("%s ", box[i][row]);  // �÷��̾ ���� �� �̾� ���
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

		// a�� b�� ����/����
		tileNode* a_prev = a->before;
		tileNode* a_next = a->next;
		tileNode* b_prev = b->before;
		tileNode* b_next = b->next;

		// a�� b�� ������ ��� ���� ó��
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
			// �Ϲ��� ��ȯ
			a->next = b_next;
			a->before = b_prev;
			b->next = a_next;
			b->before = a_prev;
		}

		// ���� ����
		a->next->before = a;
		a->before->next = a;
		b->next->before = b;
		b->before->next = b;

		return 1;
	}
	return 0;
}

void Start(tileNode* head, tileNode* start, character p[5], int player_num) {//������ Ÿ���� ������ ���
	int point;
	point = (rand() % 20) + p[player_num].loop * 10 + 80;//����Ʈ ���ް�� ������ ���
	printf("���������Դϴ�. ����Ʈ %d�� ����ϴ�.\n", point);
	return;
}

void Battle(tileNode* head, tileNode* battle, character p[5], int player_num) {//������ Ÿ���� ��Ʋ�� ���

	printf(">> ��Ʋ : ��¯����!\n");
	int input_number = 0;
	int target_player = 0;
	bool is_input_valid = false;


	//��븦 ����� �� ������ �ݺ� do-while ��
	do {

		printf("������ �ο��? | ���� ���� ���� �Է� 1 ~ 4\n");
		if (scanf("%49d", &input_number) == 1) {}
		else { while (getchar() != '\n'); }

		is_input_valid = true;

		if (input_number < 1 || input_number > 4) {
			while (getchar() != '\n');
			printf("�����̶� �ο�ڴٴ°ž�?\n�ٽ� ���\n");
			is_input_valid = false;

		}

		else {

			target_player = input_number;

			if (player_num == target_player) {
				while (getchar() != '\n');
				printf("1�� 2���̶� �ϰڴٴ� �ž�?\n�ٽ� ���\n");
				is_input_valid = false;


			}

		}


	} while (!is_input_valid);
	//���� ���� �߰��ϱ�
	Battle_w_player(head, p, player_num, target_player);

}

void judg(tileNode* head, character p[5], int player_num);

void Event(tileNode* head, tileNode* event, character p[5], int player_num) {//������ Ÿ���� �̺�Ʈ�� ���
	// �̺�Ʈ�� �� 5���������� ������ ����
	tileNode* temp = p[player_num].current_tile;
	tileNode* pre = p[player_num].current_tile;
	int dice = rand() % 3 + 1;
	printf("�̺�Ʈ �߻�!\n");
	int random = rand() % 3 + 1;

	if (dice == 1) {

		int point_point; // ��������Ʈ �߻�
		point_point = rand() % 500 + 1;
		system("cls");
		print_board(head, p);
		printf("�����߰�! ����Ʈ�� %d ��ŭ ȹ��!\n", point_point);

		p[player_num].point += point_point;

	}
	else if (dice == 2) {

		int point_point;
		point_point = rand() % 300 + 1;
		system("cls");
		print_board(head, p);
		printf("�Ҹ�ġ�⸦ ���ߴ�!\n");
		printf("����Ʈ��! %d ��ŭ �����ع��Ⱦ�...\n", point_point);

		p[player_num].point -= point_point;

	}
	else if (dice == 3) { //�������� �ڷ� 1~3ĭ �̵�
		for (int i = 0; i < random; i++) {
			temp = temp->before;
		}
	move_player(player_num, p[player_num].current_tile, temp);
	p[player_num].current_tile = temp;
	system("cls");
	print_board(head, p);
	printf("�̲�������!\n");
	printf("�ڷ� %d ĭ ��ŭ �̵��Ͽ����ϴ�.", random);
	judg(head, p, player_num);
	}

	system("pause");
	//ī�� �̱�
	CARD* draw = pop_card_from_tile_stack(pre);

	if (draw != NULL) {
		system("cls");
		print_board(head, p);

		printf("%d�÷��̾ ���������� ī�带 �̾ҽ��ϴ�.\n", player_num);
		add_card_to_player_hand(p, draw, player_num);

	}
	else {

		printf("��! �ƹ��͵� ������\n");

	}
}


void Normal(tileNode* head, tileNode* normal, character p[5], int player_num) {//������ Ÿ���� ����� ���
	int point;
	int choice;
	int cost = 250 - (250 * p[player_num].build_discount / 100);
	int upgrade_cost = 100 - (100 * p[player_num].build_discount / 100);

	point = (rand() % 20) + p[player_num].loop * 10 + 80;//����Ʈ ���ް�� ������ ���
	if (normal->build_level == 0) {//�ǹ��� ������� ����Ʈ ���� �� �Ǽ� ��ȸ
		printf("�� �Դϴ�. ����Ʈ %d�� ����ϴ�.\n ���ɺ�� : %d\n1.�����Ѵ� 2.������ȸ�� : ", point, cost);
		while (1) {
			scanf("%d", &point);
			if (point == 1) {
				if (p[player_num].point <= 250) {
					printf("����Ʈ�� �����մϴ�.\n");
					return;
				}
				else {
					normal->owner = player_num;
					normal->build_level = 1;
					p[player_num].point -= 250;
					system("cls");
					print_board(head, p);
					printf("���� �����Ͽ����ϴ�. �ǹ� ���׷��̵� ��� : %d\n 1.���׷��̵� 2.������ȸ�� : ", upgrade_cost);
					scanf("%d", &point);
					if (point == 1) {
						if (p[player_num].point <= 100) {
							printf("����Ʈ�� �����մϴ�.\n");
							return;
						}
						else {
							normal->build_level += 1;
							p[player_num].point -= 100;
							system("cls");
							print_board(head, p);
							printf("�ǹ��� ���׷��̵� �Ͽ����ϴ�. ���緹�� %d [�ִ� 3]\n", normal->build_level);
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
	else if (normal->build_level >= 1) {//�ǹ��� ������ ���� ��� ���� �ǹ��Ͻ� ���׷��̵� ���� ��� �ǹ��Ͻ� �����
		if (normal->owner == player_num) { //�ǹ��ְ� ������ �÷��̾�� ��ġ��
			if (normal->build_level <= 2) {//�ǹ� ������ 2���� ������
				printf("���׷��̵尡 �����մϴ�. �ǹ� ���׷��̵� ��� : %d\n 1.���׷��̵� 2.������ȸ�� :", upgrade_cost);
				scanf("%d", &point);
				if (point == 1) {
					if (p[player_num].point <= 100) {
						printf("����Ʈ�� �����մϴ�.\n");
						return;
					}
					else {
						normal->build_level += 1;
						p[player_num].point -= 100;
						system("cls");
						print_board(head, p);
						printf("�ǹ��� ���׷��̵� �Ͽ����ϴ�. ���緹�� %d [�ִ� 3]\n", normal->build_level);
						return;
					}
				}
				else {
					while (getchar() != '\n');return;
				}
			}
			else if (normal->build_level == 3) {//�ǹ������� �ִ�ġ�� 3�ϰ��
				printf("���μ��� �ִ뷹�� �ǹ����Դϴ�. ����Ʈ %d�� ����ϴ�.\n", point);
				return;
			}
		}
		else {//��� ������ ���� ���
			point = p[normal->owner].loop * 20 + normal->build_level * 50 + 100;//����� ���
			p[normal->owner].point += point;
			p[player_num].point -= point;
			system("cls");
			print_board(head, p);
			printf("%d �÷��̾� ������ ���� �����߽��ϴ�. ����� %d ��ŭ ���ϴ�.\n", normal->owner, point);
			if (normal->build_level <= 2) {
				printf("�μ���ȸ�� �ֽ��ϴ�. ��� %d\n 1.�μ��Ѵ� 2. ������ȸ�� : ", point);
				scanf("%d", &choice);
				while (1) {
					if (choice == 1) {
						if (p[player_num].point < point) {
							printf("�μ������ �����մϴ�.\n");
							return;
						}
						else {
							p[normal->owner].point += point;
							p[player_num].point -= point;
							normal->owner = player_num;
							system("cls");
							print_board(head, p);
							printf("%d �÷��̾� ������ ���� ����Ʈ %d�� �����ϰ� �μ��߽��ϴ�.\n 1.�ǹ� ���׷��̵� ��� : %d\n 2.������ȸ�� : ", normal->owner, point, upgrade_cost);
							scanf("%d", &point);
							if (point == 1) {
								if (p[player_num].point <= 100) {
									printf("����Ʈ�� �����մϴ�.\n");
									return;
								}
								else {
									normal->build_level += 1;
									p[player_num].point -= 100;
									system("cls");
									print_board(head, p);
									printf("�ǹ��� ���׷��̵� �Ͽ����ϴ�. ���緹�� %d [�ִ� 3]\n", normal->build_level);
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
				printf("����� ���� 3���� �̹Ƿ� �μ��� �Ұ��� �մϴ�.\n");
				return;
			}
			

		}
	}
	return;
}

void judg(tileNode* head, character p[5], int player_num) {//������ Ÿ���� �Ÿ������ �Ǵ�
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
		if (p[player_num].dice == 0 && p[player_num].fix_dice == 0) { return; } //�ֻ����� ������
		int endcount = 0;
		for (int i = 1;i <= 4;i++) {  //��� �÷��̾� �ֻ����� ������
			if (p[i].dice == 0 && p[i].fix_dice == 0) {
				endcount++;
			}
		}
		if (endcount == 4) {// �����Լ�
			return;
		}
		printf("%d �÷��̾� �����Դϴ�. �ൿ�� ������ �ּ���.\n1. �Ϲ��ֻ��� 2. Ȯ���ֻ��� 3. ī��, 4. ��ų��� 5. ��ų ���׷��̵�", player_num);
		scanf("%d", &choice);
		switch (choice) {
		case 1:
			if (p[player_num].dice == 0) {

				printf("�Ϲ� �ֻ����� �����ϴ�. �ٽ� �����ϼ���\n");

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
				printf("%d ĭ ��ŭ �̵��Ͽ����ϴ�.", choice);
				judg(head, p, player_num);
				return;
			}
			break;
		case 2:
			if (p[player_num].fix_dice == 0) {

				printf("Ȯ�� �ֻ����� �����ϴ�. �ٽ� �����ϼ���\n");

				continue;
			}
			else {
				while (1) {
					printf("Ȯ���ֻ����� �����Ͽ����ϴ�. ��ĭ �̵��ұ��? : ");
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
						printf("%d ĭ ��ŭ �̵��Ͽ����ϴ�.", choice);
						judg(head, p, player_num);
						return;
					}
					else { while (getchar() != '\n'); }
				}
			}
			break;
		case 3:
			printf("\n--- ī�� ��� ---\n");
			if (is_player_hand_empty(p, player_num)) { // �÷��̾� ���а� ����ִ��� Ȯ��
				printf("%d�� �÷��̾�� ����� �� �ִ� ī�尡 �����ϴ�!\n", player_num);
				system("pause");
				continue;

			}
			else {

				// ����� �� �ִ� �� �����ֱ�
				printf("� ī�带 ����ϰڽ��ϱ�?\n");
				show_player_card(p, player_num);

				int card_number_to_use = 0;

				if (scanf("%49d", &card_number_to_use) == 1) {}
				else { while (getchar() != '\n'); }

				CARD* card_to_use = find_card_by_index(p[player_num].card_head, card_number_to_use);
				int use_card = 1;
				if (card_to_use != NULL) {

					printf("'%s' ī�带 ����մϴ�!\n", card_to_use->card);

					if (card_to_use->card_type == 1) {

						printf(">> �ֻ��� +1�� ȿ�� �ߵ�!\n");
						p[player_num].dice += 1;
						system("pause");
					}

					else if (card_to_use->card_type == 2) {

						printf(">> ��Ʋ : ��¯����!\n");
						int input_number = 0;
						int target_player = 0;
						bool is_input_valid = false;


						//��븦 ����� �� ������ �ݺ� do-while ��
						do {

							printf("������ �ο��? | ���� ���� ���� �Է� 1 ~ 4\n");
							if (scanf("%49d", &input_number) == 1) {}
							else { while (getchar() != '\n'); }

							is_input_valid = true;

							if (input_number < 1 || input_number > 4) {

								printf("�����̶� �ο�ڴٴ°ž�?\n�ٽ� ���\n");
								is_input_valid = false;

							}

							else {

								target_player = input_number;

								if (player_num == target_player) {

									printf("1�� 2���̶� �ϰڴٴ� �ž�?\n�ٽ� ���\n");
									is_input_valid = false;


								}

							}


						} while (!is_input_valid);
						//���� ���� �߰��ϱ�
						Battle_w_player(head, p, player_num, target_player);


						//��Ʋ �Լ� ȣ��

					}

					else if (card_to_use->card_type == 3) {

						int point = 0;
						point = rand() % 500 + 1;

						printf(">> ������ �������� ȹ���մϴ� (�ִ� 500)\n");
						printf("%d ��ŭ�� ����Ʈ�� ȹ���Ͽ����ϴ�!\n", point);

						p[player_num].point += point;
						system("pause");
					}
					else if (card_to_use->card_type == 4) {
						int random = rand() % 6 + 1;
						printf("�����ϰ� �ڷ� �̵��մϴ�.\n"); //�������� �ڷ� 1~6ĭ �̵�
						for (int i = 0; i < random; i++) {
							temp = temp->before;
						}
						move_player(player_num, p[player_num].current_tile, temp);
						p[player_num].current_tile = temp;
						system("cls");
						print_board(head, p);
						printf("�ڷ� %d ĭ ��ŭ �̵��Ͽ����ϴ�.", random);
						judg(head, p, player_num);
					}
					else if (card_to_use->card_type == 5) { 
						int i = 0;
						int select = 0;
						printf("� �÷��̾ ���ִ� ���ϰ� �ٲܱ�� (event, start ��ü�Ұ�): ");
						scanf("%d", &select);
						i = swap_tiles(p[player_num].current_tile, p[select].current_tile);
						if (i == 1) { printf("��ü�� �Ϸ�Ǿ����ϴ�."); }
						else { printf("�ش� Ÿ�ϰ� ��ü�� �Ұ����մϴ�."); }
						use_card = 0;
						system("pause");
					}

				}
				if (use_card==1){ 
					CARD* removed_card = delete_card_from_player(p, card_to_use, player_num); 
					if (removed_card != NULL) { // ���������� ���Ÿ� ���� ��

						free(removed_card);
						removed_card = NULL;

					}
				}

				continue;

			}

			break;


		case 4:
			printf("\n--- ��ų ��� ---\n");

			// �ڿ� ���� ��Ʈ �����ڴ� ��ų ��� �Ұ�
			if (strstr(p[player_num].chosenSkillRoot, "�ڿ�") != NULL) {
				printf("�ڿ� ��ȭ�� �нú� ��ų�Դϴ�. ���� ����� �ʿ����� �ʽ��ϴ�.\n");
				system("pause");
				break;
			}

			// Ȯ�� �ֻ��� �� �� ���
			if (p[player_num].hasUpgradeDiceSkill) {
				if (p[player_num].diceSkillCooldown == 0) {
					printf("��ų [Ȯ�� �ֻ��� �� �� ���] ���! Ȯ�� �ֻ��� +1\n");
					p[player_num].fix_dice += 1;
					p[player_num].diceSkillCooldown = 3; // ��Ÿ�� ����
				}
				else {
					printf("��ų [Ȯ�� �ֻ��� �� �� ���]�� ���� ��Ÿ���Դϴ�. (%d�� ����)\n", p[player_num].diceSkillCooldown);
				}
			}
			else {
				printf("��ų [Ȯ�� �ֻ��� �� �� ���]�� ���� �����ϰ� ���� �ʽ��ϴ�.\n");
			}
			system("pause");
			break;

		case 5:
			printf("\n--- ��ų ���׷��̵� ---\n");

			upgradeSkill(&p[player_num]);
			system("pause");
			break;
		}


	}
}

void ending(tileNode* head, character p[5]) {
	int build_count[4][5] = {};
	int score[5] = {};
	int winner = 0; //�ְ��� �÷��̾�
	int max = 0; //�ְ���
	head = head->next;
	while (head->type != START) {
		if (head->type == NORMAL) {
			build_count[head->build_level][head->owner]++;
		}
		head = head->next;
	}
	printf("��� �÷��̾��� �ֻ����� �����Ƿ� ���� ����!\n");
	for (int i = 1;i <= 4;i++) {
		score[i] = p[i].point + build_count[1][i] * 300 + build_count[2][i] * 450 + build_count[3][i] * 600 + p[i].card_index * 200;
		winner = (max <= score[i]) ? i : winner;
		max = (max <= score[i]) ? score[i] : max;
		printf("%d�� �÷��̾� | point : %d | �ǹ� : 1����[300point] %d��, 2����[450point] %d��, 3����[600point] %d�� | ������ ī���[200point] : %d��\n �� ���� : %d ��\n", i, p[i].point, build_count[1][i], build_count[2][i], build_count[3][i], p[i].card_index, score[i]);
	}
	printf("��������� ");
	for (int i = 1;i <= 4;i++) {
		if (score[winner] == score[i]) {
			printf("%d�� �÷��̾� ", i);
		}
	}
	printf("�Դϴ�!");
	system("pause");
}


int main() {
	// 1. Ÿ�� ����
	srand(time(NULL));
	tileNode* head = set_tile(); // Ÿ�ϸ� ����
	character p[5]; //�÷��̾� ���� 0�� index�� �̻��
	for (int i = 1; i <= 4; i++) {//�÷��̾� 4�� �ʱⰪ ����
		p[i].player_num = i; //�÷��̾� ��ȣ
		p[i].loop = 0; //������
		p[i].point = 1000; //�ʱ� ����
		p[i].current_tile = head; //�÷��̾ �ִ� Ÿ��
		head->players[head->player_count++] = i; //�ʱ� ��ġ
		p[i].dice = 10; //�Ϲ��ֻ���
		p[i].fix_dice = 5; //Ȯ���ֻ���
		p[i].card_index = 0; //ī�尹��
		p[i].card_head = NULL; //
		p[i].skillTree = createSkillTree();// ĳ���� ��ų�ɷ�ġ �ʱ�ȭ
		p[i].build_discount = 0;// ������ �ʱ�ȭ
		p[i].hasUpgradeDiceSkill = false;
		p[i].hasUpgradeResourceSkill = false;
		p[i].diceSkillCooldown = 0;
		strcpy(p[i].chosenSkillRoot, "");
		p[i].skillTreeLocked = false;
		chooseSkillTree(&p[i]);

	}




	//Ÿ�� ��ġ �ٲٱ� ��� ����
	//tileNode* temp = head;
	//for (int i = 0; i < 5; i++) temp = temp->next;
	//temp->build_level = 2;
	//temp->owner = 2;
	//tileNode* temp1 = temp;
	//temp1 = temp1->before;
	//int i = swap_tiles(temp1, temp);  //4��Ÿ�ϰ� 5��Ÿ���� ��ü�� �ǹ� (�������� ������ �޾Ƽ� ��ü�� �ƴ��� �ȵƴ��� ���� ����)
	//system("cls");
	//print_board(head, p); //���� ���
	//if (i == 1) printf("��ü�Ϸ�");
	//else if (i == 0) printf("��ü�Ұ�");
	//Ÿ���� ��ŸƮ�� �̺�Ʈ Ÿ���� �����ϰ�� ��ü ����

	while (1) { //���� �÷���
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