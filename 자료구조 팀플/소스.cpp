#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
////////////////////////추가한 것 배틀 함수 구현, 게임 시작전 map만들면서 리스트를 활용한 스택으로 이벤트 카드 덱 구현, 이벤트 타일 도착 시 이벤트 후 카드덱에서 드로우 후 플레이어 카드 리스트에 추가
////////////////////////및 사용시 삭제 함수까지 구현
#define MAX_tile 24 // 타일 칸 개수는 24칸
// ---타일 타입정의
#define TILE_TYPE_START 0
#define TILE_TYPE_CONST 1 // 건설
#define TILE_TYPE_POINT 2 // 포인트
#define TILE_TYPE_EVENT 3 // 이벤트
#define TILE_TYPE_BATTLE 4 // 배틀


// 카드 보관 리스트
typedef struct CARD {

	char card[50]; // 카드 이름 생성
	int card_type; // 카드 종류 구분
	//카드  능력추가 해야할 것 같고
	//int card_num; // 카드 갯수 세기용
	CARD* before; // 전꺼
	CARD* next; // 다음꺼

}CARD;


typedef struct tileNode {

	int tile_num; // 칸 넘버
	char tile_name[30]; // 칸 이름
	int tile_type; // 타일 종류 나타내기

	int get_point; // 포인트 타일이 주는 점수
	int build_cost; // 건물 지을 때 드는 비용

	CARD* event_card_stack_top; // 플레이어 카드 뽑기

	struct tileNode* next; // 다음 노드 가리키기
}tileNode;

typedef struct CARD CARD;



//캐릭터 생성하기
typedef struct Character {

	char player_Name[100]; // 플레이어 이름 
	int point; // 점수
	int current_tile_index; // 타일의 번호
	tileNode* current_tile; // 타일 노드와 플레이어 간의 상호작용
	CARD* card_head; // 소지한 카드
	int card_index; // 카드 개수 출력
	int dice; // 일반 주사위 개수
	int fix_dice; // 확정 주사위 개수
	int roll_dice; //주사위 굴리기

}Character;

tileNode* map_head = NULL; // 맵 출발지 설정

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

	printf("캐릭터의 정보가 설정되었습니다.\n");

}

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

void add_card_to_player_hand(Character* character, CARD* new_card) {

	new_card->before = NULL; // 새 헤드의 before는 NULL (OK)

	new_card->next = character->card_head;

	character->card_head = new_card; // 새로 추가된 카드는 머리가 됨
	printf("%s 손패에 '%s' 카드를 추가했다.!\n", character->player_Name, new_card->card);

	//character->card_head->card_num += 1;

}

int show_player_card(Character* character) {

	CARD* current = character->card_head;
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

CARD* delete_card_from_player(Character* character, CARD* card_remove) {

	CARD* current = character->card_head;

	while (current != NULL) {

		if (current == card_remove) {

			if (current->before != NULL) {

				current->before->next = current->next; // 제거할 카드 빼놓고 연결하기

			}
			else { // 카드가 하나 남았을 떄

				character->card_head = current->next;

			}

			if (current->next != NULL) {

				current->next->before = current->before; // 

			}

			current->before = NULL;
			current->next = NULL;

			printf("%s 님의 손패에서 '%s' 카드를 제거 했습니다.\n", character->player_Name, current->card);
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

void Battle_w_player(Character* character1, Character* character2) {

	int dice1 = 0;
	int dice2 = 0;
	int pointpoint = rand() % 2000 + 1;

	printf("%s 가 %s 에게 배틀을 신청했습니다!\n굴려라!!!! 운명의 다이스 롤!!!\n", character1, character2);
	do {

		dice1 = rand() % 6 + 1;
		dice2 = rand() % 6 + 1;
		printf("%s는 %d 가 나왔습니다!\n", character1->player_Name, dice1);
		printf("%s는 %d 가 나왔습니다!\n", character2->player_Name, dice2);

		if (dice1 == dice2) {

			printf("뭐야? 동점이야? 다시 굴려!!!!!!!\n");

		}

	} while (dice1 == dice2);

	if (dice1 > dice2) {

		printf("%s 가 승리했습니다. %s 로부터 일정량의 포인트(%d)를 빼앗습니다!\n", character1, character2, pointpoint);
		character1->point += pointpoint;
		character2->point -= pointpoint;

	}

	else if (dice2 > dice1) {

		printf("%s 가 승리했습니다. %s 로부터 일정량의 포인트(%d)를 빼앗습니다!\n", character2, character1, pointpoint);
		character1->point -= pointpoint;
		character2->point += pointpoint;

	}


}

void build_event(Character* character) {



}

void event_Event(Character* character) {

	// 이벤트는 총 5가지정도로 구성할 예정
	character->roll_dice = rand() % 3 + 1;

	printf("이벤트 발생!\n");

	if (character->roll_dice == 1) {

		int point_point; // 랜덤포인트 발생
		point_point = rand() % 2000 + 1;
		printf("포인트 줘요 -> 포인트를 %d 만큼 획득!\n", point_point);

		character->point += point_point;

	}


	else if (character->roll_dice == 2) {

		int point_point;
		point_point = rand() % 1500 + 1;
		printf("어이쿠 저런 ㅋㅋㅋ 벼락이 쳤습니다.\n");
		printf("포인트가! %d 만큼 감소해버렸어...\n", point_point);

		character->point -= point_point;

	}


	else if (character->roll_dice == 3) {

		printf("뭐야 시봉방 아무것도 없네..흑..\n");
		printf("아무것도 없었다... 갈 길 가자\n");

	}

}





void map_create() {

	tileNode* current_node = NULL; // 맵 만들게 포인터 하나 만들어서 끝까지 가게끔 해 맵 생성

	map_head = (tileNode*)malloc(sizeof(tileNode)); // tileNode만큼의 크기 할당
	if (map_head == NULL) {

		fprintf(stderr, "오류 : 맵 생성 실패\n");
		exit(EXIT_FAILURE);

	}

	memset(map_head, 0, sizeof(tileNode)); // 메모리 0으로 초기화
	map_head->tile_num = 0; // 처음 맵번호는 0번이야
	strcpy(map_head->tile_name, "시작점");
	map_head->tile_type = TILE_TYPE_START;
	current_node = map_head; // 현재 타일과 current를 동일시 해줌

	for (int i = 1; i < MAX_tile; i++) {

		tileNode* newNode = (tileNode*)malloc(sizeof(tileNode));
		if (newNode == NULL) {

			printf("에러났어요\n");
			exit(1);

		}

		memset(newNode, 0, sizeof(tileNode)); // 메모리를 0으로 초기화
		newNode->get_point = 0;
		newNode->build_cost = 0;

		newNode->tile_num = i;
		// 각 타일 마다의 이름 붙이기
		if (i == 2 || i == 4 || i == 7 || i == 8 || i == 11 || i == 13 || i == 16 || i == 21) {

			strcpy(newNode->tile_name, "점수 타일");
			newNode->tile_type = TILE_TYPE_POINT;
			newNode->get_point = 1000;


		}

		else if (i == 1 || i == 6 || i == 10 || i == 14 || i == 19) {

			strcpy(newNode->tile_name, "건설 타일");
			newNode->tile_type = TILE_TYPE_CONST;

			newNode->build_cost = 1000;

		}

		else if (i == 3 || i == 9 || i == 15 || i == 17 || i == 22) {

			strcpy(newNode->tile_name, "이벤트 타일");
			newNode->tile_type = TILE_TYPE_EVENT;

			printf("이벤트 타일에 카드를 채웁니다....\n");

			for (int k = 0; k < 10; k++) {

				CARD* card_to_add = (CARD*)malloc(sizeof(CARD));

				int numnum; // 랜덤으로 스택에 채움
				numnum = rand() % 3 + 1;

				if (numnum == 1) {

					strcpy(card_to_add->card, "주사위 한개 추가");
					card_to_add->card_type = 1;
					//card_to_add->card_num = 0; // 카드 개수 세기 필요 없겠다
					card_to_add->before = NULL;
					card_to_add->next = NULL;
					push_card_to_stack(newNode, card_to_add);

				}

				if (numnum == 2) {

					strcpy(card_to_add->card, "배틀 카드 추가");
					card_to_add->card_type = 2;
					//card_to_add->card_num = 0;
					card_to_add->before = NULL;
					card_to_add->next = NULL;
					push_card_to_stack(newNode, card_to_add);

				}

				if (numnum == 3) {

					strcpy(card_to_add->card, "랜덤 점수 얻기 카드");
					card_to_add->card_type = 3;
					//card_to_add->card_num = 0;
					card_to_add->before = NULL;
					card_to_add->next = NULL;
					push_card_to_stack(newNode, card_to_add);


				}

			}

		}

		else if (i == 5 || i == 12 || i == 18 || i == 20) {

			strcpy(newNode->tile_name, "배틀 타일");
			newNode->tile_type = TILE_TYPE_BATTLE;

		}

		current_node->next = newNode; // 다음칸으로 이어주기
		current_node = newNode; // 현재 가리키고 있는 노드 최신꺼로 옮겨주기

	}

	current_node->next = map_head;
	printf("게임을 할 모든 준비를 마쳤습니다. (총 타일 수 %d)\n", MAX_tile);

}

void map_free() {

	if (map_head == NULL) {

		printf("맵이 비었습니다.\n");
		return;

	}

	// 리스트에 한개의 노드만 있을 때
	if (map_head->next == map_head) {

		free(map_head);
		map_head = NULL;
		printf("완료\n");
		return;

	}

	tileNode* d_current = map_head; //너가 free할거야
	tileNode* next_node; //free해줄 꺼 다음꺼를 가리킴

	tileNode* last_node = map_head;
	while (last_node->next != map_head) {

		last_node = last_node->next;

	}

	last_node->next = NULL; // 선형 노드로 만들어서 쉽게 free하기

	while (d_current != NULL) {

		next_node = d_current->next;
		free(d_current);
		d_current = next_node;

	}
	// 돌아다니면서 free 다 해줌

	map_head = NULL; // 노드 다 제거 했으니까 head도 free

	printf("정리 완료\n");

}

void show_status(Character* character) {

	character->current_tile_index = character->current_tile_index % 24;
	printf("이게 너의 현재 상황이야\n");
	printf("나의 현재 위치는? : %d번째 타일 : %s \n", character->current_tile_index, character->current_tile->tile_name);
	printf("플레이어 이름 : %s 소지 point : %d ", character->player_Name, character->point);

	int show_card_many = show_player_card(character) - 1;

	if (character->card_head != NULL) {

		printf("카드 소지 개수 : %d ", show_card_many);

	}
	else {

		printf("카드 소지 개수 : 낫띵 없어요... ");

	}

	printf("일반 주사위 개수 : %d 확정 주사위 개수 : %d\n", character->dice, character->fix_dice);

}

int turn(Character* character) {

	character->roll_dice = rand() % 6 + 1;
	//printf("%s 이(가) 주사위를 굴렸다! %d이 나왔습니다.\n", character->player_Name, character->roll_dice);

	return character->roll_dice;

}
// 순서 정하기
Character* user_sequence(Character* players[], int num_players) {


	for (int i = 0; i < num_players; i++) {

		turn(players[i]); // 주사위 굴리기
		printf("%s 님이 주사위를 굴렸습니다! 주사위 눈금 : %d\n", players[i]->player_Name, players[i]->roll_dice);

	}

	// 주사위 가장 높은 수 찾기
	int max_roll = -1;
	for (int i = 0; i < num_players; ++i) {

		if (players[i]->roll_dice > max_roll) {

			max_roll = players[i]->roll_dice;

		}

	}


	// 가장 높은 플레이어들 다시 굴리기
	Character* compare_player_dice[4];
	int compare_count = 0;
	for (int i = 0; i < num_players; ++i) {

		if (players[i]->roll_dice == max_roll) {

			compare_player_dice[compare_count++] = players[i]; // 동점자들 배열에 저장하기

		}

	}

	// 동점자 확인하기
	if (compare_count == 1) {

		printf("%s 의 선공!\n", compare_player_dice[0]->player_Name);
		return compare_player_dice[0];

	}
	else {

		//동점자가 있다.
		printf("동점자다! 동점자만 다시 굴립니다.!\n");

		return user_sequence(compare_player_dice, compare_count);

	}

	/*
	if (character1->roll_dice < character2->roll_dice) {

		printf("%s 의 선공입니다.\n", character2->player_Name);
		return character2;

	}

	else if (character1->roll_dice > character2->roll_dice) {

		printf("%s 의 선공입니다.\n", character1->player_Name);
		return character1;

	}

	else {

		printf("동점! 다시 굴립니다.\n");
		user_sequence(character1, character2);

	}*/

}
// 게임 진행하기
void game_play(Character* players[], int num_players) {


	//순서 정하기

	Character* first_player = user_sequence(players, num_players);

	int current_player_index = 0;
	for (int i = 0; i < num_players; i++)
	{

		if (players[i] == first_player) {

			current_player_index = i;
			break;

		}

	}


	//int num_players = 4; // 플레이어수

	/*if (first_player == character1) {

		players[0] = character1;
		players[1] = character2;

	}

	else {

		players[0] = character2;
		players[1] = character1;

	}*/
	// 플레이어 순서 인덱스

	Character* currentPlayer = (Character*)malloc(sizeof(Character));


	while (true) {


		currentPlayer = players[current_player_index];

		int choice = 0;

		//턴이 끝났다는 걸 알리는
		bool turn_ended = false;
		// 게임 끝났는지 판단하기
		int order_end = 0;

		for (int i = 0; i < num_players; i++) {

			if ((players[i]->dice == 0) && (players[i]->fix_dice == 0)) {

				order_end += 1;

			}

		}

		if (order_end == 4) {

			printf("게임이 종료되었습니다. 결산창으로 이동합니다.\n");
			break;

		}

		if (currentPlayer->dice == 0 && currentPlayer->fix_dice == 0) {

			turn_ended = true;

		}


		while (!turn_ended) {

			printf("%s의 차례 무엇을 할까? : 1. 주사위 굴리기 2. 카드 사용 3. 현재 상황 보기\n", currentPlayer->player_Name);
			scanf("%49d", &choice);

			if (choice == 1) {

				int select_dice = 0;

				printf("무슨 주사위를 쓸까?\n");
				printf("1. 일반 주사위 2. 확정 주사위\n");
				scanf("%49d", &select_dice);

				if (select_dice == 1) {

					if (currentPlayer->dice == 0) {

						printf("가지고 있는 주사위가 없어.. 다시 선택하자\n");
						continue;
					}

					// 주사위 값 저장
					int dice_num;
					dice_num = turn(currentPlayer);
					printf("%d 만큼 이동할게요!\n", dice_num);
					for (int i = 0; i < dice_num; i++) {
						currentPlayer->current_tile = currentPlayer->current_tile->next;
						currentPlayer->current_tile_index += 1;


					}


					currentPlayer->dice -= 1;
					turn_ended = true;

				}

				else if (select_dice == 2) {

					if (currentPlayer->fix_dice == 0) {

						printf("가지고 있는 확정주사위가 없어... 다시 선택하자\n");
						continue;

					}

					int fixdice_num = 0;

					printf("몇 만큼 이동을 할까? (1~6을 입력해주세요)\n");
					scanf("%49d", &fixdice_num);

					if (fixdice_num < 7 && fixdice_num > 0) {

						printf("%d 만큼 이동할게요!\n", fixdice_num);

						for (int i = 0; i < fixdice_num; i++) {

							currentPlayer->current_tile = currentPlayer->current_tile->next;
							currentPlayer->current_tile_index += 1;


						}

						currentPlayer->fix_dice -= 1;
						turn_ended = true;
					}

					else {

						printf("잘못 입력되었습니다. 다시 입력하세요\n");
						continue;

					}



				}

				else {

					printf("잘못 입력되었습니다. 다시 입력하세요\n");
					continue;

				}



				printf("현재 위치는 %s 이야! 느낌 좋은데?\n", currentPlayer->current_tile->tile_name);

				switch (currentPlayer->current_tile->tile_type) {
				case TILE_TYPE_POINT:
					printf("%s 타일에 도착! %d 포인트를 얻었습니다!\n", currentPlayer->current_tile->tile_name, 1000);
					currentPlayer->point += currentPlayer->current_tile->get_point;
					printf("현재 포인트 : %d\n", currentPlayer->point);
					break;
				case TILE_TYPE_CONST:
					printf("%s 타일에 도착!\n", currentPlayer->current_tile->tile_name);
					//건설 로직 추가하기
					break;
				case TILE_TYPE_EVENT:
					printf("%s 타일에 도착! 이벤트가 발생합니다.\n", currentPlayer->current_tile->tile_name);

					//이벤트 만들어 추가하기
					event_Event(currentPlayer);
					//이벤트 추가 효과
					printf("이벤트 타일 추가 효과 발동!\n");

					{
						//카드 뽑기
						CARD* draw = pop_card_from_tile_stack(currentPlayer->current_tile);

						if (draw != NULL) {

							printf("%s가 성공적으로 카드를 뽑았습니다.\n", currentPlayer->player_Name);
							add_card_to_player_hand(currentPlayer, draw);

						}
						else {

							printf("꽝! 아무것도 없지롱\n");

						}
					}

					break;
				case TILE_TYPE_BATTLE:
				{
					int input_number = 0;
					int target_player = -1;
					bool is_input_valid = false;

					printf("%s 타일에 도착! 전투를 준비하세요!\n", currentPlayer->current_tile->tile_name);

					//상대를 제대로 고를 때까지 반복 do-while 문
					do {

						printf("누구랑 싸울까? | 본인 순서 제외 입력 1 ~ 4\n");
						scanf("%49d", &input_number);

						is_input_valid = true;

						if (input_number < 1 || input_number > 4) {

							printf("유령이랑 싸우겠다는거야?\n다시 골라\n");
							is_input_valid = false;

						}

						else {

							target_player = input_number - 1;



							if (strcmp(currentPlayer->player_Name, players[target_player]->player_Name) == 0) {

								printf("1인 2역이라도 하겠다는 거야?\n다시 골라\n");
								is_input_valid = false;


							}

						}


					} while (!is_input_valid);
					//전투 로직 추가하기
					Battle_w_player(currentPlayer, players[target_player]);
				}

				break;
				case TILE_TYPE_START:
					// 구현할 수도 있고 안할 수도 있습니다.
					break;
				default:
					printf("뭐야 오류잖아!\n");
					break;




				}



			}

			if (choice == 2) {


				printf("\n--- 카드 사용 ---\n");
				if (is_player_hand_empty(currentPlayer)) { // 플레이어 손패가 비어있는지 확인
					printf("%s 님은 사용할 수 있는 카드가 없습니다!\n", currentPlayer->player_Name);

					continue;

				}
				else {

					// 사용할 수 있는 거 보여주기
					printf("어떤 카드를 사용하겠습니까?\n");
					show_player_card(currentPlayer);

					int card_number_to_use = 0;

					scanf("%49d", &card_number_to_use);

					CARD* card_to_use = find_card_by_index(currentPlayer->card_head, card_number_to_use);

					if (card_to_use != NULL) {

						printf("'%s' 카드를 사용합니다!\n", card_to_use->card);

						if (card_to_use->card_type == 1) {

							printf(">> 주사위 +1의 효과 발동!\n");
							currentPlayer->dice += 1;

						}

						else if (card_to_use->card_type == 2) {

							printf(">> 배틀 : 맞짱뜨자!\n");
							int input_number = 0;
							int target_player = -1;
							bool is_input_valid = false;

							printf("%s 타일에 도착! 전투를 준비하세요!\n", currentPlayer->current_tile->tile_name);

							//상대를 제대로 고를 때까지 반복 do-while 문
							do {

								printf("누구랑 싸울까? | 본인 순서 제외 입력 1 ~ 4\n");
								scanf("%49d", &input_number);

								is_input_valid = true;

								if (input_number < 1 || input_number > 4) {

									printf("유령이랑 싸우겠다는거야?\n다시 골라\n");
									is_input_valid = false;

								}

								else {

									target_player = input_number - 1;

									if (strcmp(currentPlayer->player_Name, players[target_player]->player_Name) == 0) {

										printf("1인 2역이라도 하겠다는 거야?\n다시 골라\n");
										is_input_valid = false;


									}

								}


							} while (!is_input_valid);
							//전투 로직 추가하기
							Battle_w_player(currentPlayer, players[target_player]);


							//배틀 함수 호출

						}

						else if (card_to_use->card_type == 3) {

							int point = 0;
							point = rand() % 2000 + 1;

							printf(">> 점수를 무작위로 획득합니다 (최대 2000)\n");
							printf("%d 만큼의 포인트를 획득하였습니다!\n", point);

							currentPlayer->point += point;

						}

					}

					CARD* removed_card = delete_card_from_player(currentPlayer, card_to_use);

					if (removed_card != NULL) { // 성공적으로 제거를 했을 시

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

			printf("저런... 현재 플레이어의 주사위는 없습니다!\n");

		}*/

		printf("%s 의 턴 종료 다음 차례로 넘어갑니다.\n", currentPlayer->player_Name);
		current_player_index = (current_player_index + 1) % num_players;



	}



}


int main() {
	srand(time(NULL));
	//캐릭터 생성
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

			fprintf(stderr, "오류 : 플레이어 생성실패!\n");
			exit(EXIT_FAILURE);

		}

		setCharacter(players[i]);

		printf("플레이어 %d의 이름을 입력하세요 : ", i + 1);
		scanf("%49s", players[i]->player_Name);

		printf("플레이어 %s 생성완료 (점수 : %d, 일반 주사위 : %d, 확정 주사위 : %d)\n", players[i]->player_Name, players[i]->point, players[i]->dice, players[i]->fix_dice);


	}










	//카드 포인터 만들어주기
	CARD* card;




	// 게임 들어가기 전

	/*setCharacter(players[0]);
	setCharacter(players[1]);
	setCharacter(players[2]);
	setCharacter(players[3]);*/

	// 플레이어 이름 생성
	/*printf("플레이어1의 이름을 입력하세요 : ");
	scanf("%s", character1->player_Name);
	printf("플레이어 %s 생성 완료 (점수 : %d, 일반 주사위 : %d, 확정 주사위 : %d)\n", character1->player_Name, character1->point, character1->dice, character1->fix_dice);



	printf("플레이어2의 이름을 입력하세요 : ");
	scanf("%s", character2->player_Name);
	printf("플레이어 %s 생성완료 (점수 : %d, 일반 주사위 : %d, 확정 주사위 : %d)\n", character2->player_Name, character2->point, character2->dice, character2->fix_dice);


	printf("플레이어3의 이름을 입력하세요 : ");
	scanf("%s", character3->player_Name);
	printf("플레이어 %s 생성완료 (점수 : %d, 일반 주사위 : %d, 확정 주사위 : %d)\n", character3->player_Name, character3->point, character3->dice, character3->fix_dice);

	printf("플레이어4의 이름을 입력하세요 : ");
	scanf("%s", character4->player_Name);
	printf("플레이어 %s 생성완료 (점수 : %d, 일반 주사위 : %d, 확정 주사위 : %d)\n", character4->player_Name, character4->point, character4->dice, character4->fix_dice);
	*/

	//게임 들어가기
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