#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
////////////////////////�߰��� �� ��Ʋ �Լ� ����, ���� ������ map����鼭 ����Ʈ�� Ȱ���� �������� �̺�Ʈ ī�� �� ����, �̺�Ʈ Ÿ�� ���� �� �̺�Ʈ �� ī�嵦���� ��ο� �� �÷��̾� ī�� ����Ʈ�� �߰�
////////////////////////�� ���� ���� �Լ����� ����
#define MAX_tile 24 // Ÿ�� ĭ ������ 24ĭ
// ---Ÿ�� Ÿ������
#define TILE_TYPE_START 0
#define TILE_TYPE_CONST 1 // �Ǽ�
#define TILE_TYPE_POINT 2 // ����Ʈ
#define TILE_TYPE_EVENT 3 // �̺�Ʈ
#define TILE_TYPE_BATTLE 4 // ��Ʋ


// ī�� ���� ����Ʈ
typedef struct CARD {

	char card[50]; // ī�� �̸� ����
	int card_type; // ī�� ���� ����
	//ī��  �ɷ��߰� �ؾ��� �� ����
	//int card_num; // ī�� ���� �����
	CARD* before; // ����
	CARD* next; // ������

}CARD;


typedef struct tileNode {

	int tile_num; // ĭ �ѹ�
	char tile_name[30]; // ĭ �̸�
	int tile_type; // Ÿ�� ���� ��Ÿ����

	int get_point; // ����Ʈ Ÿ���� �ִ� ����
	int build_cost; // �ǹ� ���� �� ��� ���

	CARD* event_card_stack_top; // �÷��̾� ī�� �̱�

	struct tileNode* next; // ���� ��� ����Ű��
}tileNode;

typedef struct CARD CARD;



//ĳ���� �����ϱ�
typedef struct Character {

	char player_Name[100]; // �÷��̾� �̸� 
	int point; // ����
	int current_tile_index; // Ÿ���� ��ȣ
	tileNode* current_tile; // Ÿ�� ���� �÷��̾� ���� ��ȣ�ۿ�
	CARD* card_head; // ������ ī��
	int card_index; // ī�� ���� ���
	int dice; // �Ϲ� �ֻ��� ����
	int fix_dice; // Ȯ�� �ֻ��� ����
	int roll_dice; //�ֻ��� ������

}Character;

tileNode* map_head = NULL; // �� ����� ����

void setCard(CARD* cards) {

	cards->before = NULL;
	cards->next = NULL;
	//cards->card_num = 0;
	strcpy(cards->card, "");

}

void setCharacter(Character* character) {

	memset(character->player_Name, 0, sizeof(character->player_Name));;
	character->point = 500;
	character->current_tile_index = 0;
	character->current_tile = map_head;
	character->card_head = NULL;
	//character->card_index = 0;
	character->dice = 3;
	character->fix_dice = 3;
	character->roll_dice = 0;

	printf("ĳ������ ������ �����Ǿ����ϴ�.\n");

}

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

void add_card_to_player_hand(Character* character, CARD* new_card) {

	new_card->before = NULL; // �� ����� before�� NULL (OK)

	new_card->next = character->card_head;

	character->card_head = new_card; // ���� �߰��� ī��� �Ӹ��� ��
	printf("%s ���п� '%s' ī�带 �߰��ߴ�.!\n", character->player_Name, new_card->card);

	//character->card_head->card_num += 1;

}

int show_player_card(Character* character) {

	CARD* current = character->card_head;
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

CARD* delete_card_from_player(Character* character, CARD* card_remove) {

	CARD* current = character->card_head;

	while (current != NULL) {

		if (current == card_remove) {

			if (current->before != NULL) {

				current->before->next = current->next; // ������ ī�� ������ �����ϱ�

			}
			else { // ī�尡 �ϳ� ������ ��

				character->card_head = current->next;

			}

			if (current->next != NULL) {

				current->next->before = current->before; // 

			}

			current->before = NULL;
			current->next = NULL;

			printf("%s ���� ���п��� '%s' ī�带 ���� �߽��ϴ�.\n", character->player_Name, current->card);
			return current;

		}

		current = current->next;

	}


}

bool is_player_hand_empty(Character* character) {

	if (character == NULL || character->card_head == NULL) {

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

void Battle_w_player(Character* character1, Character* character2) {

	int dice1 = 0;
	int dice2 = 0;
	int pointpoint = rand() % 2000 + 1;

	printf("%s �� %s ���� ��Ʋ�� ��û�߽��ϴ�!\n������!!!! ����� ���̽� ��!!!\n", character1, character2);
	do {

		dice1 = rand() % 6 + 1;
		dice2 = rand() % 6 + 1;
		printf("%s�� %d �� ���Խ��ϴ�!\n", character1->player_Name, dice1);
		printf("%s�� %d �� ���Խ��ϴ�!\n", character2->player_Name, dice2);

		if (dice1 == dice2) {

			printf("����? �����̾�? �ٽ� ����!!!!!!!\n");

		}

	} while (dice1 == dice2);

	if (dice1 > dice2) {

		printf("%s �� �¸��߽��ϴ�. %s �κ��� �������� ����Ʈ(%d)�� ���ѽ��ϴ�!\n", character1, character2, pointpoint);
		character1->point += pointpoint;
		character2->point -= pointpoint;

	}

	else if (dice2 > dice1) {

		printf("%s �� �¸��߽��ϴ�. %s �κ��� �������� ����Ʈ(%d)�� ���ѽ��ϴ�!\n", character2, character1, pointpoint);
		character1->point -= pointpoint;
		character2->point += pointpoint;

	}


}

void build_event(Character* character) {



}

void event_Event(Character* character) {

	// �̺�Ʈ�� �� 5���������� ������ ����
	character->roll_dice = rand() % 3 + 1;

	printf("�̺�Ʈ �߻�!\n");

	if (character->roll_dice == 1) {

		int point_point; // ��������Ʈ �߻�
		point_point = rand() % 2000 + 1;
		printf("����Ʈ ��� -> ����Ʈ�� %d ��ŭ ȹ��!\n", point_point);

		character->point += point_point;

	}


	else if (character->roll_dice == 2) {

		int point_point;
		point_point = rand() % 1500 + 1;
		printf("������ ���� ������ ������ �ƽ��ϴ�.\n");
		printf("����Ʈ��! %d ��ŭ �����ع��Ⱦ�...\n", point_point);

		character->point -= point_point;

	}


	else if (character->roll_dice == 3) {

		printf("���� �ú��� �ƹ��͵� ����..��..\n");
		printf("�ƹ��͵� ������... �� �� ����\n");

	}

}





void map_create() {

	tileNode* current_node = NULL; // �� ����� ������ �ϳ� ���� ������ ���Բ� �� �� ����

	map_head = (tileNode*)malloc(sizeof(tileNode)); // tileNode��ŭ�� ũ�� �Ҵ�
	if (map_head == NULL) {

		fprintf(stderr, "���� : �� ���� ����\n");
		exit(EXIT_FAILURE);

	}

	memset(map_head, 0, sizeof(tileNode)); // �޸� 0���� �ʱ�ȭ
	map_head->tile_num = 0; // ó�� �ʹ�ȣ�� 0���̾�
	strcpy(map_head->tile_name, "������");
	map_head->tile_type = TILE_TYPE_START;
	current_node = map_head; // ���� Ÿ�ϰ� current�� ���Ͻ� ����

	for (int i = 1; i < MAX_tile; i++) {

		tileNode* newNode = (tileNode*)malloc(sizeof(tileNode));
		if (newNode == NULL) {

			printf("���������\n");
			exit(1);

		}

		memset(newNode, 0, sizeof(tileNode)); // �޸𸮸� 0���� �ʱ�ȭ
		newNode->get_point = 0;
		newNode->build_cost = 0;

		newNode->tile_num = i;
		// �� Ÿ�� ������ �̸� ���̱�
		if (i == 2 || i == 4 || i == 7 || i == 8 || i == 11 || i == 13 || i == 16 || i == 21) {

			strcpy(newNode->tile_name, "���� Ÿ��");
			newNode->tile_type = TILE_TYPE_POINT;
			newNode->get_point = 1000;


		}

		else if (i == 1 || i == 6 || i == 10 || i == 14 || i == 19) {

			strcpy(newNode->tile_name, "�Ǽ� Ÿ��");
			newNode->tile_type = TILE_TYPE_CONST;

			newNode->build_cost = 1000;

		}

		else if (i == 3 || i == 9 || i == 15 || i == 17 || i == 22) {

			strcpy(newNode->tile_name, "�̺�Ʈ Ÿ��");
			newNode->tile_type = TILE_TYPE_EVENT;

			printf("�̺�Ʈ Ÿ�Ͽ� ī�带 ä��ϴ�....\n");

			for (int k = 0; k < 10; k++) {

				CARD* card_to_add = (CARD*)malloc(sizeof(CARD));

				int numnum; // �������� ���ÿ� ä��
				numnum = rand() % 3 + 1;

				if (numnum == 1) {

					strcpy(card_to_add->card, "�ֻ��� �Ѱ� �߰�");
					card_to_add->card_type = 1;
					//card_to_add->card_num = 0; // ī�� ���� ���� �ʿ� ���ڴ�
					card_to_add->before = NULL;
					card_to_add->next = NULL;
					push_card_to_stack(newNode, card_to_add);

				}

				if (numnum == 2) {

					strcpy(card_to_add->card, "��Ʋ ī�� �߰�");
					card_to_add->card_type = 2;
					//card_to_add->card_num = 0;
					card_to_add->before = NULL;
					card_to_add->next = NULL;
					push_card_to_stack(newNode, card_to_add);

				}

				if (numnum == 3) {

					strcpy(card_to_add->card, "���� ���� ��� ī��");
					card_to_add->card_type = 3;
					//card_to_add->card_num = 0;
					card_to_add->before = NULL;
					card_to_add->next = NULL;
					push_card_to_stack(newNode, card_to_add);


				}

			}

		}

		else if (i == 5 || i == 12 || i == 18 || i == 20) {

			strcpy(newNode->tile_name, "��Ʋ Ÿ��");
			newNode->tile_type = TILE_TYPE_BATTLE;

		}

		current_node->next = newNode; // ����ĭ���� �̾��ֱ�
		current_node = newNode; // ���� ����Ű�� �ִ� ��� �ֽŲ��� �Ű��ֱ�

	}

	current_node->next = map_head;
	printf("������ �� ��� �غ� ���ƽ��ϴ�. (�� Ÿ�� �� %d)\n", MAX_tile);

}

void map_free() {

	if (map_head == NULL) {

		printf("���� ������ϴ�.\n");
		return;

	}

	// ����Ʈ�� �Ѱ��� ��常 ���� ��
	if (map_head->next == map_head) {

		free(map_head);
		map_head = NULL;
		printf("�Ϸ�\n");
		return;

	}

	tileNode* d_current = map_head; //�ʰ� free�Ұž�
	tileNode* next_node; //free���� �� �������� ����Ŵ

	tileNode* last_node = map_head;
	while (last_node->next != map_head) {

		last_node = last_node->next;

	}

	last_node->next = NULL; // ���� ���� ���� ���� free�ϱ�

	while (d_current != NULL) {

		next_node = d_current->next;
		free(d_current);
		d_current = next_node;

	}
	// ���ƴٴϸ鼭 free �� ����

	map_head = NULL; // ��� �� ���� �����ϱ� head�� free

	printf("���� �Ϸ�\n");

}

void show_status(Character* character) {

	character->current_tile_index = character->current_tile_index % 24;
	printf("�̰� ���� ���� ��Ȳ�̾�\n");
	printf("���� ���� ��ġ��? : %d��° Ÿ�� : %s \n", character->current_tile_index, character->current_tile->tile_name);
	printf("�÷��̾� �̸� : %s ���� point : %d ", character->player_Name, character->point);

	int show_card_many = show_player_card(character) - 1;

	if (character->card_head != NULL) {

		printf("ī�� ���� ���� : %d ", show_card_many);

	}
	else {

		printf("ī�� ���� ���� : ���� �����... ");

	}

	printf("�Ϲ� �ֻ��� ���� : %d Ȯ�� �ֻ��� ���� : %d\n", character->dice, character->fix_dice);

}

int turn(Character* character) {

	character->roll_dice = rand() % 6 + 1;
	//printf("%s ��(��) �ֻ����� ���ȴ�! %d�� ���Խ��ϴ�.\n", character->player_Name, character->roll_dice);

	return character->roll_dice;

}
// ���� ���ϱ�
Character* user_sequence(Character* players[], int num_players) {


	for (int i = 0; i < num_players; i++) {

		turn(players[i]); // �ֻ��� ������
		printf("%s ���� �ֻ����� ���Ƚ��ϴ�! �ֻ��� ���� : %d\n", players[i]->player_Name, players[i]->roll_dice);

	}

	// �ֻ��� ���� ���� �� ã��
	int max_roll = -1;
	for (int i = 0; i < num_players; ++i) {

		if (players[i]->roll_dice > max_roll) {

			max_roll = players[i]->roll_dice;

		}

	}


	// ���� ���� �÷��̾�� �ٽ� ������
	Character* compare_player_dice[4];
	int compare_count = 0;
	for (int i = 0; i < num_players; ++i) {

		if (players[i]->roll_dice == max_roll) {

			compare_player_dice[compare_count++] = players[i]; // �����ڵ� �迭�� �����ϱ�

		}

	}

	// ������ Ȯ���ϱ�
	if (compare_count == 1) {

		printf("%s �� ����!\n", compare_player_dice[0]->player_Name);
		return compare_player_dice[0];

	}
	else {

		//�����ڰ� �ִ�.
		printf("�����ڴ�! �����ڸ� �ٽ� �����ϴ�.!\n");

		return user_sequence(compare_player_dice, compare_count);

	}

	/*
	if (character1->roll_dice < character2->roll_dice) {

		printf("%s �� �����Դϴ�.\n", character2->player_Name);
		return character2;

	}

	else if (character1->roll_dice > character2->roll_dice) {

		printf("%s �� �����Դϴ�.\n", character1->player_Name);
		return character1;

	}

	else {

		printf("����! �ٽ� �����ϴ�.\n");
		user_sequence(character1, character2);

	}*/

}
// ���� �����ϱ�
void game_play(Character* players[], int num_players) {


	//���� ���ϱ�

	Character* first_player = user_sequence(players, num_players);

	int current_player_index = 0;
	for (int i = 0; i < num_players; i++)
	{

		if (players[i] == first_player) {

			current_player_index = i;
			break;

		}

	}


	//int num_players = 4; // �÷��̾��

	/*if (first_player == character1) {

		players[0] = character1;
		players[1] = character2;

	}

	else {

		players[0] = character2;
		players[1] = character1;

	}*/
	// �÷��̾� ���� �ε���

	Character* currentPlayer = (Character*)malloc(sizeof(Character));


	while (true) {


		currentPlayer = players[current_player_index];

		int choice = 0;

		//���� �����ٴ� �� �˸���
		bool turn_ended = false;
		// ���� �������� �Ǵ��ϱ�
		int order_end = 0;

		for (int i = 0; i < num_players; i++) {

			if ((players[i]->dice == 0) && (players[i]->fix_dice == 0)) {

				order_end += 1;

			}

		}

		if (order_end == 4) {

			printf("������ ����Ǿ����ϴ�. ���â���� �̵��մϴ�.\n");
			break;

		}

		if (currentPlayer->dice == 0 && currentPlayer->fix_dice == 0) {

			turn_ended = true;

		}


		while (!turn_ended) {

			printf("%s�� ���� ������ �ұ�? : 1. �ֻ��� ������ 2. ī�� ��� 3. ���� ��Ȳ ����\n", currentPlayer->player_Name);
			scanf("%49d", &choice);

			if (choice == 1) {

				int select_dice = 0;

				printf("���� �ֻ����� ����?\n");
				printf("1. �Ϲ� �ֻ��� 2. Ȯ�� �ֻ���\n");
				scanf("%49d", &select_dice);

				if (select_dice == 1) {

					if (currentPlayer->dice == 0) {

						printf("������ �ִ� �ֻ����� ����.. �ٽ� ��������\n");
						continue;
					}

					// �ֻ��� �� ����
					int dice_num;
					dice_num = turn(currentPlayer);
					printf("%d ��ŭ �̵��ҰԿ�!\n", dice_num);
					for (int i = 0; i < dice_num; i++) {
						currentPlayer->current_tile = currentPlayer->current_tile->next;
						currentPlayer->current_tile_index += 1;


					}


					currentPlayer->dice -= 1;
					turn_ended = true;

				}

				else if (select_dice == 2) {

					if (currentPlayer->fix_dice == 0) {

						printf("������ �ִ� Ȯ���ֻ����� ����... �ٽ� ��������\n");
						continue;

					}

					int fixdice_num = 0;

					printf("�� ��ŭ �̵��� �ұ�? (1~6�� �Է����ּ���)\n");
					scanf("%49d", &fixdice_num);

					if (fixdice_num < 7 && fixdice_num > 0) {

						printf("%d ��ŭ �̵��ҰԿ�!\n", fixdice_num);

						for (int i = 0; i < fixdice_num; i++) {

							currentPlayer->current_tile = currentPlayer->current_tile->next;
							currentPlayer->current_tile_index += 1;


						}

						currentPlayer->fix_dice -= 1;
						turn_ended = true;
					}

					else {

						printf("�߸� �ԷµǾ����ϴ�. �ٽ� �Է��ϼ���\n");
						continue;

					}



				}

				else {

					printf("�߸� �ԷµǾ����ϴ�. �ٽ� �Է��ϼ���\n");
					continue;

				}



				printf("���� ��ġ�� %s �̾�! ���� ������?\n", currentPlayer->current_tile->tile_name);

				switch (currentPlayer->current_tile->tile_type) {
				case TILE_TYPE_POINT:
					printf("%s Ÿ�Ͽ� ����! %d ����Ʈ�� ������ϴ�!\n", currentPlayer->current_tile->tile_name, 1000);
					currentPlayer->point += currentPlayer->current_tile->get_point;
					printf("���� ����Ʈ : %d\n", currentPlayer->point);
					break;
				case TILE_TYPE_CONST:
					printf("%s Ÿ�Ͽ� ����!\n", currentPlayer->current_tile->tile_name);
					//�Ǽ� ���� �߰��ϱ�
					break;
				case TILE_TYPE_EVENT:
					printf("%s Ÿ�Ͽ� ����! �̺�Ʈ�� �߻��մϴ�.\n", currentPlayer->current_tile->tile_name);

					//�̺�Ʈ ����� �߰��ϱ�
					event_Event(currentPlayer);
					//�̺�Ʈ �߰� ȿ��
					printf("�̺�Ʈ Ÿ�� �߰� ȿ�� �ߵ�!\n");

					{
						//ī�� �̱�
						CARD* draw = pop_card_from_tile_stack(currentPlayer->current_tile);

						if (draw != NULL) {

							printf("%s�� ���������� ī�带 �̾ҽ��ϴ�.\n", currentPlayer->player_Name);
							add_card_to_player_hand(currentPlayer, draw);

						}
						else {

							printf("��! �ƹ��͵� ������\n");

						}
					}

					break;
				case TILE_TYPE_BATTLE:
				{
					int input_number = 0;
					int target_player = -1;
					bool is_input_valid = false;

					printf("%s Ÿ�Ͽ� ����! ������ �غ��ϼ���!\n", currentPlayer->current_tile->tile_name);

					//��븦 ����� �� ������ �ݺ� do-while ��
					do {

						printf("������ �ο��? | ���� ���� ���� �Է� 1 ~ 4\n");
						scanf("%49d", &input_number);

						is_input_valid = true;

						if (input_number < 1 || input_number > 4) {

							printf("�����̶� �ο�ڴٴ°ž�?\n�ٽ� ���\n");
							is_input_valid = false;

						}

						else {

							target_player = input_number - 1;



							if (strcmp(currentPlayer->player_Name, players[target_player]->player_Name) == 0) {

								printf("1�� 2���̶� �ϰڴٴ� �ž�?\n�ٽ� ���\n");
								is_input_valid = false;


							}

						}


					} while (!is_input_valid);
					//���� ���� �߰��ϱ�
					Battle_w_player(currentPlayer, players[target_player]);
				}

				break;
				case TILE_TYPE_START:
					// ������ ���� �ְ� ���� ���� �ֽ��ϴ�.
					break;
				default:
					printf("���� �����ݾ�!\n");
					break;




				}



			}

			if (choice == 2) {


				printf("\n--- ī�� ��� ---\n");
				if (is_player_hand_empty(currentPlayer)) { // �÷��̾� ���а� ����ִ��� Ȯ��
					printf("%s ���� ����� �� �ִ� ī�尡 �����ϴ�!\n", currentPlayer->player_Name);

					continue;

				}
				else {

					// ����� �� �ִ� �� �����ֱ�
					printf("� ī�带 ����ϰڽ��ϱ�?\n");
					show_player_card(currentPlayer);

					int card_number_to_use = 0;

					scanf("%49d", &card_number_to_use);

					CARD* card_to_use = find_card_by_index(currentPlayer->card_head, card_number_to_use);

					if (card_to_use != NULL) {

						printf("'%s' ī�带 ����մϴ�!\n", card_to_use->card);

						if (card_to_use->card_type == 1) {

							printf(">> �ֻ��� +1�� ȿ�� �ߵ�!\n");
							currentPlayer->dice += 1;

						}

						else if (card_to_use->card_type == 2) {

							printf(">> ��Ʋ : ��¯����!\n");
							int input_number = 0;
							int target_player = -1;
							bool is_input_valid = false;

							printf("%s Ÿ�Ͽ� ����! ������ �غ��ϼ���!\n", currentPlayer->current_tile->tile_name);

							//��븦 ����� �� ������ �ݺ� do-while ��
							do {

								printf("������ �ο��? | ���� ���� ���� �Է� 1 ~ 4\n");
								scanf("%49d", &input_number);

								is_input_valid = true;

								if (input_number < 1 || input_number > 4) {

									printf("�����̶� �ο�ڴٴ°ž�?\n�ٽ� ���\n");
									is_input_valid = false;

								}

								else {

									target_player = input_number - 1;

									if (strcmp(currentPlayer->player_Name, players[target_player]->player_Name) == 0) {

										printf("1�� 2���̶� �ϰڴٴ� �ž�?\n�ٽ� ���\n");
										is_input_valid = false;


									}

								}


							} while (!is_input_valid);
							//���� ���� �߰��ϱ�
							Battle_w_player(currentPlayer, players[target_player]);


							//��Ʋ �Լ� ȣ��

						}

						else if (card_to_use->card_type == 3) {

							int point = 0;
							point = rand() % 2000 + 1;

							printf(">> ������ �������� ȹ���մϴ� (�ִ� 2000)\n");
							printf("%d ��ŭ�� ����Ʈ�� ȹ���Ͽ����ϴ�!\n", point);

							currentPlayer->point += point;

						}

					}

					CARD* removed_card = delete_card_from_player(currentPlayer, card_to_use);

					if (removed_card != NULL) { // ���������� ���Ÿ� ���� ��

						free(removed_card);
						removed_card = NULL;

					}

					continue;

				}

				break;

			}

			if (choice == 3) {

				show_status(currentPlayer);

			}

		}

		/*if (currentPlayer->dice == 0 && currentPlayer->fix_dice == 0) {

			printf("����... ���� �÷��̾��� �ֻ����� �����ϴ�!\n");

		}*/

		printf("%s �� �� ���� ���� ���ʷ� �Ѿ�ϴ�.\n", currentPlayer->player_Name);
		current_player_index = (current_player_index + 1) % num_players;



	}



}


int main() {
	srand(time(NULL));
	//ĳ���� ����
	int num_players = 4;
	Character* players[4];

	/*Character* character1;
	Character* character2;
	Character* character3;
	Character* character4;
	character1 = (Character*)malloc(sizeof(Character));
	character2 = (Character*)malloc(sizeof(Character));
	character3 = (Character*)malloc(sizeof(Character));
	character4 = (Character*)malloc(sizeof(Character));*/

	map_create();

	for (int i = 0; i < num_players; i++) {

		players[i] = (Character*)malloc(sizeof(Character));
		if (players[i] == NULL) {

			fprintf(stderr, "���� : �÷��̾� ��������!\n");
			exit(EXIT_FAILURE);

		}

		setCharacter(players[i]);

		printf("�÷��̾� %d�� �̸��� �Է��ϼ��� : ", i + 1);
		scanf("%49s", players[i]->player_Name);

		printf("�÷��̾� %s �����Ϸ� (���� : %d, �Ϲ� �ֻ��� : %d, Ȯ�� �ֻ��� : %d)\n", players[i]->player_Name, players[i]->point, players[i]->dice, players[i]->fix_dice);


	}










	//ī�� ������ ������ֱ�
	CARD* card;




	// ���� ���� ��

	/*setCharacter(players[0]);
	setCharacter(players[1]);
	setCharacter(players[2]);
	setCharacter(players[3]);*/

	// �÷��̾� �̸� ����
	/*printf("�÷��̾�1�� �̸��� �Է��ϼ��� : ");
	scanf("%s", character1->player_Name);
	printf("�÷��̾� %s ���� �Ϸ� (���� : %d, �Ϲ� �ֻ��� : %d, Ȯ�� �ֻ��� : %d)\n", character1->player_Name, character1->point, character1->dice, character1->fix_dice);



	printf("�÷��̾�2�� �̸��� �Է��ϼ��� : ");
	scanf("%s", character2->player_Name);
	printf("�÷��̾� %s �����Ϸ� (���� : %d, �Ϲ� �ֻ��� : %d, Ȯ�� �ֻ��� : %d)\n", character2->player_Name, character2->point, character2->dice, character2->fix_dice);


	printf("�÷��̾�3�� �̸��� �Է��ϼ��� : ");
	scanf("%s", character3->player_Name);
	printf("�÷��̾� %s �����Ϸ� (���� : %d, �Ϲ� �ֻ��� : %d, Ȯ�� �ֻ��� : %d)\n", character3->player_Name, character3->point, character3->dice, character3->fix_dice);

	printf("�÷��̾�4�� �̸��� �Է��ϼ��� : ");
	scanf("%s", character4->player_Name);
	printf("�÷��̾� %s �����Ϸ� (���� : %d, �Ϲ� �ֻ��� : %d, Ȯ�� �ֻ��� : %d)\n", character4->player_Name, character4->point, character4->dice, character4->fix_dice);
	*/

	//���� ����
	//user_sequence(character1, character2);
	game_play(players, num_players);


	map_free();


	for (int i = 0; i < num_players; ++i) {

		if (players[i] != NULL) {

			free_player_card(players[i]->card_head);
			free(players[i]);

		}

	}

	return 0;

}