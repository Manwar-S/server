#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/ip.h>
#include<stdio.h>
#include<unistd.h>
#include<vector>
#include<thread>
#include<cstring>
#include<mutex>
#include"lurk_pro_server.h"
#include"lurk_classes.h"
#include<fstream>
#include<sstream>
#include<iostream>
#include<iterator>
#include<signal.h>
using namespace std;
using std:: vector;
Game G;
extern vector<Player*> Players;//
extern vector<Player*> Mine;//
int Conn_check = 0;
vector<Room> myRooms;
vector<Character> Monsters;
extern vector<int> conn;
vector <string> temp;
vector <string> temp2;
void Create_Rooms(){
	fstream fin;
	fin.open("Rooms.csv", ios:: in);

	string line, word, temp2;
	int roll;
	
	while (fin.good()) {
		getline(fin, line); 
		stringstream s(line);
	
		while (getline(s, word, '%')) { 
   
            		temp.push_back(word);
			if(temp.size() == 3){
				Room  m;
				m.Room_num = stoi(temp[0]);
				m.Room_name = temp[1];
				m.Room_des = temp[2];
				m.Room_des_len = m.Room_des.length()-1;
				temp.clear();
				myRooms.push_back(m);
			} 
        	} 
 	}

	myRooms[0].conn = {1, 2, 3};
	myRooms[1].conn = {4, 9, 10};
	myRooms[2].conn = {0, 8, 11};
	myRooms[3].conn = {0};
	myRooms[4].conn = {1, 5};
	myRooms[5].conn = {4, 6};
	myRooms[6].conn = {7, 5};
	myRooms[7].conn = {6};
	myRooms[8].conn = {2};
	myRooms[9].conn = {1};
	myRooms[10].conn = {1};
	myRooms[11].conn = {2};


	fin.close();
}

void Create_Monsters(){
        fstream fin;
        fin.open("Monsters.csv", ios:: in);

        string line, word;
       // int roll;
        
        while (fin.good()) {
                getline(fin, line);
                stringstream s(line);
                while (getline(s, word, ',')) {
			temp2.push_back(word); 
                        if(temp2.size() == 8){
                                Character m;
				m.Char_Name = temp2[0];
				m.Flags = 248;
				m.Attack = stoi(temp2[1]);
				m.Defense = stoi(temp2[2]);
				m.Regen = stoi(temp2[3]);
				m.Health = stoi(temp2[4]);
				m.Gold = stoi(temp2[5]);
				m.Curr_Room_Num = stoi(temp2[6]);
				m.Char_des = temp2[7];
				m.Char_Des_len = m.Char_des.length()-1;
                                temp2.clear();
                                Monsters.push_back(m);
                        } 
                } 
        }

	fin.close();
}

//13 write
void writeConnection(Connection P, int skt){
	int type = 13;
	write(skt,&type,1);

	write(skt, &P.Conn_Room_num, 2); 

        char name_buff[sizeof(P.Conn_Room_name)];
        strncpy(name_buff,P.Conn_Room_name.c_str(), sizeof(name_buff));
        write(skt, name_buff,32);

        write(skt, &P.Conn_Room_des_len,2);
        char des_buff[P.Conn_Room_des_len];
        memcpy(des_buff, P.Conn_Room_des.c_str(), P.Conn_Room_des_len);
        write(skt, des_buff, P.Conn_Room_des_len);



}
//11 write
void writeGame(Game G, int skt){
	int type = 11;
	write(skt,&type,1);

	write(skt, &G.Int_points,2);
        write(skt, &G.Stat_limit,2);

	unsigned short des_len = G.Game_des.length();
        char des_buff[des_len];
        write(skt, &des_len,2);
        memcpy(des_buff, G.Game_des.c_str(), des_len);
        write(skt, des_buff, des_len);





}

// 10 write
void writeCharacter(Character Ch , int skt){
	
	int type = 10;
        write(skt,&type,1);
        char namebuff[33];
	strncpy(namebuff,Ch.Char_Name.c_str(),sizeof(namebuff));
	namebuff[sizeof(namebuff) - 1] = 0;
        write(skt, namebuff,32);
        write(skt, &Ch.Flags,1);
        write(skt, &Ch.Attack,2);
        write(skt, &Ch.Defense,2);
        write(skt, &Ch.Regen,2);
        write(skt, &Ch.Health,2);
        write(skt, &Ch.Gold,2);
        write(skt, &Ch.Curr_Room_Num,2);

        write(skt, &Ch.Char_Des_len,2);
	char des_buff[Ch.Char_Des_len];
        memcpy(des_buff, Ch.Char_des.c_str(), Ch.Char_Des_len);
        write(skt, des_buff, Ch.Char_Des_len);

}

// 9 write
void writeRoom(Room P, int skt){
	int type = 9;
        write(skt,&type,1);

	write(skt, &P.Room_num, 2); 

	char name_buff[sizeof(P.Room_name)];
        strncpy(name_buff,P.Room_name.c_str(), sizeof(name_buff));
        write(skt, name_buff,32);

	write(skt, &P.Room_des_len,2);
        char des_buff[P.Room_des_len];
        memcpy(des_buff, P.Room_des.c_str(), P.Room_des_len);
        write(skt, des_buff, P.Room_des_len);


}

// 8 write
void writeAccept(Accept P, int skt){

	int type = 8;
        write(skt,&type,1);
	

	write(skt, &P.Accepted, 1);

}

// 7 write
void writeError(Error P, int skt){

	int type = 7;
        write(skt,&type,1);

	if(P.Err_code == 0){
		P.Err_Message = "Other.";
	}
	else if(P.Err_code == 1){
                P.Err_Message = "Bad Room.";
        }
	else if(P.Err_code == 2){
                P.Err_Message = "Player Exists.";
        }
	else if(P.Err_code == 3){
                P.Err_Message = "Bad Monster.";
        }
	else if(P.Err_code == 4){
                P.Err_Message = "Stat Error.";
        }
	else if(P.Err_code == 5){
                P.Err_Message = "Not Ready.";
        }
	else if(P.Err_code == 6){
                P.Err_Message = "No target.";
        }
	else if(P.Err_code == 7){
                P.Err_Message = "No Fight.";
        }
	else if(P.Err_code == 8){
                P.Err_Message = "No player vs. player combat on the server.";
        }
	else{
	}
	write(skt, &P.Err_code, 1);
	P.Err_Message_len = P.Err_Message.length();
	write(skt, &P.Err_Message_len, 2);
	
	char des_buff[P.Err_Message_len];
        memcpy(des_buff, P.Err_Message.c_str(), P.Err_Message_len);
        write(skt, des_buff, P.Err_Message_len);

}

// 1 write

void writeMessage(Message P, int skt){

	int type = 1;
        write(skt,&type,1);

	write(skt, &P.Message_Len, 2);
	char rbuff[33];
	strncpy(rbuff,P.Rec_Name.c_str(),sizeof(rbuff));
        rbuff[sizeof(rbuff) - 1] = 0;
	write(skt,rbuff,32);


	char sbuff[33];
        strncpy(sbuff,P.Sen_Name.c_str(),sizeof(sbuff));
        rbuff[sizeof(sbuff) - 1] = 0;
	write(skt,sbuff ,32);

	char des_buff[P.Message_Len];
        memcpy(des_buff, P.The_Message.c_str(), P.Message_Len);
        write(skt, des_buff, P.Message_Len);
}

void conn_check(int sig){
	cout << "Errorrr" << endl;
	Conn_check = 1;
}




class client;
vector<client*> clients;
mutex data_lock; 
class client {
        public:
                int client_fd;
                thread client_thread;
                char name[128];
                int c_index;
                mutex send_mutex;
		//int type;
		Player* mine;//
		Player me;//
		int size = Players.size();

                client(int cfd) : client_fd(cfd) {
                        client_thread = thread(&client::our_thread, this); 
                }
                void our_thread(){
			Character Ch;
			Character mook;
			Connection Co;
			Room R;
			Accept A;
			Error E;
			Loot L;
			PvPFight Pf;
			Fight F;
			ChangeRoom Cr;
			Message M;
			int alive, join, monster, started, ready;
			alive  = 128;
                        monster = 32;
                        started = 16;
                        join = 64;
			ready = 8;
                        char this_buffer[2048];
                        char name[128];
			int type;
			writeGame(G,client_fd);//
			//vector<int> r;
			struct sigaction sig;
			sig.sa_handler = conn_check;
			sigaction(SIGPIPE, &sig, 0);


                        for(;;){
				read(client_fd, &type,1);//
                                data_lock.lock();

                                if(type == 10){
					cout << "READING A TYPE 10" << endl;
					char namebuff[33];
                                        read(client_fd, namebuff,32);
					namebuff[32] = 0;//
					Ch.Char_Name = (namebuff);
					cout << "Name: " << Ch.Char_Name << endl;

                                        read(client_fd, &Ch.Flags,1);
					Ch.Flags =  ready;
                                        read(client_fd, &Ch.Attack,2);
                                        read(client_fd, &Ch.Defense,2);
                                        read(client_fd, &Ch.Regen,2);
                                        read(client_fd, &Ch.Health,2);
					Ch.Health = 100;
                                        read(client_fd, &Ch.Gold,2);
					Ch.Gold = 0;
                                        read(client_fd, &Ch.Curr_Room_Num,2);
					Ch.Curr_Room_Num = 0;
                                        char des_buff[Ch.Char_Des_len];
                                        read(client_fd, &Ch.Char_Des_len,2);
                                        read(client_fd, des_buff,Ch.Char_Des_len);
					Ch.Char_des = des_buff;

					if (Ch.Attack+Ch.Defense+Ch.Regen > 100 || Ch.Attack+Ch.Defense+Ch.Regen  <= 0){
                                                E.Err_code = 4;
                                                writeError(E,client_fd);

                                        }
					 else{
                                                me.setName(Ch.Char_Name);
                                                me.setFlags(Ch.Flags);
                                                me.setAtt(Ch.Attack);
                                                me.setDef(Ch.Defense);
                                                me.setReg(Ch.Regen);
                                                me.setHP(Ch.Health);
                                                me.setGold(Ch.Gold);
                                                me.setRoom(Ch.Curr_Room_Num);
                                                me.setDes(Ch.Char_des);
						me.Ch.Char_Des_len = Ch.Char_des.length();
                                                mine->AddChar(&me);  
                                                writeAccept(A,client_fd);
						writeCharacter(Ch,client_fd);
                                        }
					int size = Players.size();
					for( int i = 0; i < Players.size(); i++){//
						//cout << "Chacking player name: " << endl;
						/*if (Ch.Char_Name == Players[i]->getName()){
							E.Err_code = 2;
                                                	writeError(E,client_fd);
							cout << "Found one!" << endl;
						}*/
							//writeCharacter(Ch, client_fd);
							//writeAccept(A,client_fd);


					}

				}
				if(type == 8){//
					cout << "READING A TYPE 8" << endl;
					read(client_fd,&A.Accepted,1);
					writeAccept(A,client_fd);
                                }
				if(type == 6 && me.getFlags() != 216){
					cout << "READING A TYPE 6" << endl;
					Ch.Flags = started + alive + join;
					Ch.Curr_Room_Num = 1;
                                        me.setFlags(Ch.Flags);
                                        me.setRoom(Ch.Curr_Room_Num);
					R.Room_num = Ch.Curr_Room_Num;
                                        R.Room_name = myRooms[0].getRname();
                                        R.Room_des = myRooms[0].getRdes();
                                        R.Room_des_len = myRooms[0].Room_des_len;
                                        R.setRnum(R.Room_num);
                                        R.setRname(R.Room_name);
                                        R.setRdes(R.Room_des);
                                        writeRoom(R,client_fd);
					for(int j = 0; j < myRooms[0].conn.size(); j++){                 
                                                Co.Conn_Room_num = myRooms[myRooms[0].conn[j]].getRnum();
                                                Co.Conn_Room_name = myRooms[myRooms[0].conn[j]].getRname();
                                                Co.Conn_Room_des = myRooms[myRooms[0].conn[j]].getRdes();
                                                Co.Conn_Room_des_len = myRooms[myRooms[0].conn[j]].Room_des_len; 
                                                writeConnection(Co, client_fd);
                                        }
					for(int j = client_fd; j >= 4; j--){
						for (int i = 0 ; i < Players.size(); i++){
							if(me.getRoom() == Players[i]->getRoom()){
								writeCharacter(Players[i]->getPlayer(), j);
								cout << "Current value of socket: " << client_fd << endl;
								cout << "Client socket: " << client_fd << endl;
                                                        	cout << "Everyone has been displayed?" << endl;
                                                       		size++;
							}
						}
                                        }

					if(size > Players.size()){
						for (int i = 0 ; i <  Players.size(); i++){
							if( me.getRoom() == Players[i]->getRoom()){
                                                		writeCharacter(Players[i]->getPlayer(),client_fd);
								cout << "Skt: "  << Players[i]->getSkt() << endl;
                                                		cout << "hello" << endl;
							}
                                         	}
					
					}
                                }
				if(type == 5){
					cout << "READING A TYPE 5" << endl;
					char nbuff[33];
                                        read(client_fd, &nbuff,32);
                                        nbuff[32] = 0;
                                        L.Loot_Name = (nbuff);
					int gold;
					if( me.getHP() > 0){
						for( int k = 0; k < Monsters.size(); k++){
							if(L.Loot_Name == Monsters[k].getMName() && me.getRoom() == Monsters[k].getRoom()){
								if( Monsters[k].Health <= 0){
									if(Monsters[k].Gold > 0){
										cout << "Monster looted: " << Monsters[k].getMName() << endl;
										gold = Monsters[k].getGold();
										me.setGold((me.getGold()+gold));
										Monsters[k].Gold = 0;
										mook.Char_Name = Monsters[k].getMName();
                                                                                mook.Flags = Monsters[k].Flags;
                                                                                mook.Attack = Monsters[k].getAtt();
                                                                                mook.Defense = Monsters[k].getDef();
                                                                                mook.Regen = Monsters[k].getReg();
                                                                                mook.Health = Monsters[k].Health;
                                                                                mook.Gold =Monsters[k].getGold();
                                                                                mook.Curr_Room_Num =Monsters[k].getRoom();
                                                                                mook.Char_des =  Monsters[k].getDes();
                                                                                mook.Char_Des_len = Monsters[k].Char_Des_len;
										writeCharacter(mook, client_fd);
									}
								}
							}
						}
						for( int n = 0; n < Players.size(); n++){
							 if(L.Loot_Name == Players[n]->getName() && me.getRoom() == Players[n]->getRoom()){
								if( Players[n]->getHP() <= 0){
									if(Players[n]->getGold() > 0){
										cout << "Player looted: " << Players[n]->getName() << endl;
                                                                                gold = Players[n]->getGold();
                                                                                me.setGold((me.getGold()+gold));
                                                                                Players[n]->setGold(0);
									}

								}
							}





						}
						for(int j = client_fd; j >= 4; j--){
                                                        for (int i = 0 ; i < Players.size(); i++){
                                                                writeCharacter(Players[i]->getPlayer(), j);                                                        
                                                        }

                                                }

                                                if(size > Players.size()){

                                                        for (int i = 0 ; i <  Players.size(); i++){
                                                                writeCharacter(Players[i]->getPlayer(),client_fd);
                                                        }
                                                }

					}
                                }
				if(type == 4){
					cout << "READING A TYPE 4" << endl;
					E.Err_code = 8;
                                        writeError(E,client_fd);
				}
				if(type == 3){
					//Character mook;
					cout << "READING A TYPE 3" << endl;
					int Patt =0;
					int Pdef = 0;
					int Matt = 0;
					int Mdef = 0;
					for( int k = 0; k < Monsters.size(); k++){
						for( int n = 0; n < Players.size(); n++){
							if(Players[n]->getFlags() != 88 &&  Players[n]->getRoom() == Monsters[k].getRoom()){
										for( int p = 0; p < Players.size(); p++){
											Patt += Players[p]->getAtt();
											cout << "Player DMG:  " << Patt << endl;
										}
										Matt += Monsters[k].getAtt();
										cout << "Player DMG:  " << Matt << endl;
	                                                        		Mdef += Monsters[k].getDef();
										if( Patt - (Monsters[k].Defense/2 ) <= 0){
											Patt = 0;
											Monsters[k].Health -= Patt;
										}
										else{
											cout << "Damage being dealt: " << (Patt - Monsters[k].Defense/2) << endl;
											Monsters[k].Health -= Patt - (Monsters[k].Defense/2);
											cout <<  "Monster: " << Monsters[k].Char_Name << endl;
											cout <<  "Monsters HP: " << Monsters[k].Health << endl;
											Patt = 0;

										}
										if( Monsters[k].Health <= 0){
											Monsters[k].Health = 0;
											Monsters[k].Flags = 120;
										}
										if (Players[n]->getRoom() == Monsters[k].getRoom()){
											mook.Char_Name = Monsters[k].getMName();
                                                                        		mook.Flags = Monsters[k].Flags;
                                                                        		mook.Attack = Monsters[k].getAtt();
                                                                        		mook.Defense = Monsters[k].getDef();
                                                                        		mook.Regen = Monsters[k].getReg();
                                                                        		mook.Health = Monsters[k].Health;
                                                                        		mook.Gold =Monsters[k].getGold();
                                                                        		mook.Curr_Room_Num =Monsters[k].getRoom();
                                                                        		mook.Char_des =  Monsters[k].getDes();
                                                                        		mook.Char_Des_len = Monsters[k].Char_Des_len;
                                                                        		writeCharacter(mook, client_fd);
										}
										if(Monsters[k].Health > 0){
											if( Matt > 0){
												int  Pain = (Matt - (Players[n]->getDef()/2));
												cout << "PAAAAAIN: " << Pain << endl;
												int Hp = Players[n]->getHP() - Pain;
												Players[n]->setHP(Hp);
												cout << "HP: " << Players[n]->getHP() << endl;  
											}
										}
										if(Players[n]->getHP() <= 0){
											Players[n]->setHP(0);
											Players[n]->setFlags(88);
										}
							}
							if(Players[n]->getRoom() == Monsters[k].getRoom() && (Monsters[k].Health <= 0 || Monsters[k].Flags == 120) ){
								E.Err_code = 7;
                                        			writeError(E,client_fd);
							}

						}
						for(int j = client_fd; j >= 4; j--){
                                                        for (int i = 0 ; i < Players.size(); i++){
                                                                writeCharacter(Players[i]->getPlayer(), j);                                                        
							}

                                                }

                                                if(size > Players.size()){

                                                	for (int i = 0 ; i <  Players.size(); i++){
                                                        	writeCharacter(Players[i]->getPlayer(),client_fd);
                                                        }
                                                }
						
					}
                                }
				if(type == 2){
					cout << "READING A TYPE 2" << endl;
					read(client_fd,&Cr.change_room,2);
					for( int i = 0; i < myRooms.size(); i++){
						if(Cr.change_room == myRooms[i].getRnum()){
							if(Ch.Curr_Room_Num  == 9 && Cr.change_room == 3){
								Ch.Curr_Room_Num = Cr.change_room;
								me.setRoom(Ch.Curr_Room_Num);
								int Hp = me.getHP() + me.getReg();
                                                                me.setHP(Hp);
                                                                if( me.getHP() > 100){
                                                                        me.setHP(100);
                                                                }
                                                                R.Room_num = Cr.change_room;
                                                                R.Room_name = myRooms[i].getRname();
                                                                R.Room_des = myRooms[i].getRdes();
                                                                R.Room_des_len = myRooms[i].Room_des_len;
								R.setRnum(R.Room_num);
                                                                R.setRname(R.Room_name);
                                                                R.setRdes(R.Room_des);
                                                                writeRoom(R,client_fd);
								for(int j = 0; j < myRooms[i].conn.size(); j++){                 
                                                			Co.Conn_Room_num = myRooms[myRooms[i].conn[j]].getRnum();
                                                			Co.Conn_Room_name = myRooms[myRooms[i].conn[j]].getRname();
                                                			Co.Conn_Room_des = myRooms[myRooms[i].conn[j]].getRdes();
                                                			Co.Conn_Room_des_len = myRooms[myRooms[i].conn[j]].Room_des_len; 
                                                			writeConnection(Co, client_fd);
								}

								 for(int j = client_fd; j >= 4; j--){
                                                			for (int i = 0 ; i < Players.size(); i++){
                                                        			writeCharacter(Players[i]->getPlayer(), j);
                                                			}
                                        			}

                                        			if(size > Players.size()){

                                                			for (int i = 0 ; i <  Players.size(); i++){
                                                				writeCharacter(Players[i]->getPlayer(),client_fd);
                                                				}
                                        			}

 							}
							else if(Ch.Curr_Room_Num == 3 && (Cr.change_room == 9 || Cr.change_room == 12 || Cr.change_room == 1)){
                                                                Ch.Curr_Room_Num = Cr.change_room;
								me.setRoom(Ch.Curr_Room_Num);
								int Hp = me.getHP() + me.getReg();
                                                                me.setHP(Hp);
                                                                if( me.getHP() > 100){
                                                                        me.setHP(100);
                                                                }
                                                                R.Room_num = Cr.change_room;
                                                                R.Room_name = myRooms[i].getRname();
                                                                R.Room_des = myRooms[i].getRdes();
                                                                R.Room_des_len = myRooms[i].Room_des_len;
								R.setRnum(R.Room_num);
                                                                R.setRname(R.Room_name);
                                                                R.setRdes(R.Room_des);
                                                                writeRoom(R,client_fd);
								for(int j = 0; j < myRooms[i].conn.size(); j++){                 
                                                                        Co.Conn_Room_num = myRooms[myRooms[i].conn[j]].getRnum();
                                                                        Co.Conn_Room_name = myRooms[myRooms[i].conn[j]].getRname();
                                                                        Co.Conn_Room_des = myRooms[myRooms[i].conn[j]].getRdes();
                                                                        Co.Conn_Room_des_len = myRooms[myRooms[i].conn[j]].Room_des_len; 
                                                                        writeConnection(Co, client_fd);
                                                                }  
								for( int k = 0; k < Monsters.size(); k++){
									if (Monsters[k].getRoom() == Ch.Curr_Room_Num){
									mook.Char_Name = Monsters[k].getMName();
                                                                        mook.Flags = Monsters[k].getFlags();
                                                                        mook.Attack = Monsters[k].getAtt();
                                                                        mook.Defense = Monsters[k].getDef();
                                                                        mook.Regen = Monsters[k].getReg();
                                                                        mook.Health = Monsters[k].getHP();
                                                                        mook.Gold = Monsters[k].getGold();
                                                                        mook.Curr_Room_Num = Monsters[k].getRoom();
                                                                        mook.Char_des = Monsters[k].getDes();
                                                                        mook.Char_Des_len = Monsters[k].Char_Des_len;
                                                                        writeCharacter(mook,client_fd);
									}
								}
								for(int j = client_fd; j >= 4; j--){
                                                                        for (int i = 0 ; i < Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(), j);
                                                                        }
                                                                }

                                                                if(size > Players.size()){

                                                                        for (int i = 0 ; i <  Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(),client_fd);
									}            
                                                                }
                                                        }
							else if(Ch.Curr_Room_Num == 1 && (Cr.change_room == 3 || Cr.change_room == 4 || Cr.change_room == 2)){
                                                                Ch.Curr_Room_Num = Cr.change_room;
								me.setRoom(Ch.Curr_Room_Num);
								int Hp = me.getHP() + me.getReg();
                                                                me.setHP(Hp);
                                                                if( me.getHP() > 100){
                                                                        me.setHP(100);
                                                                }
                                                                R.Room_num = Cr.change_room;
                                                                R.Room_name = myRooms[i].getRname();
                                                                R.Room_des = myRooms[i].getRdes();
                                                                R.Room_des_len = myRooms[i].Room_des_len;
								R.setRnum(R.Room_num);
                                                                R.setRname(R.Room_name);
                                                                R.setRdes(R.Room_des);
                                                                writeRoom(R,client_fd);
								for(int j = 0; j < myRooms[i].conn.size(); j++){                 
                                                                        Co.Conn_Room_num = myRooms[myRooms[i].conn[j]].getRnum();
                                                                        Co.Conn_Room_name = myRooms[myRooms[i].conn[j]].getRname();
                                                                        Co.Conn_Room_des = myRooms[myRooms[i].conn[j]].getRdes();
                                                                        Co.Conn_Room_des_len = myRooms[myRooms[i].conn[j]].Room_des_len; 
                                                                        writeConnection(Co, client_fd);
                                                                }  

								for( int k = 0; k < Monsters.size(); k++){
                                                                        if (Monsters[k].getRoom() == Ch.Curr_Room_Num){
                                                                        mook.Char_Name = Monsters[k].getMName();
                                                                        mook.Flags = Monsters[k].getFlags();
                                                                        mook.Attack = Monsters[k].getAtt();
                                                                        mook.Defense = Monsters[k].getDef();
                                                                        mook.Regen = Monsters[k].getReg();
                                                                        mook.Health = Monsters[k].getHP();
                                                                        mook.Gold = Monsters[k].getGold();
                                                                        mook.Curr_Room_Num = Monsters[k].getRoom();
                                                                        mook.Char_des = Monsters[k].getDes();
                                                                        mook.Char_Des_len = Monsters[k].Char_Des_len;
                                                                        writeCharacter(mook,client_fd);
                                                                        }
                                                                }
								 for(int j = client_fd; j >= 4; j--){
                                                                        for (int i = 0 ; i < Players.size(); i++){
										if(me.getRoom() == Players[i]->getRoom()){
                                                                                writeCharacter(Players[i]->getPlayer(), j);
										}
                                                                        }

                                                                }

                                                                if(size > Players.size()){

                                                                        for (int i = 0 ; i <  Players.size(); i++){
										if(me.getRoom() == Players[i]->getRoom()){
                                                                			writeCharacter(Players[i]->getPlayer(),client_fd);
                                                        			}
                                                                        }
                                                                }
                                                        }
							else if(Ch.Curr_Room_Num == 12 && Cr.change_room == 3){
                                                                Ch.Curr_Room_Num = Cr.change_room;
								me.setRoom(Ch.Curr_Room_Num);
								int Hp = me.getHP() + me.getReg();
                                                                me.setHP(Hp);
                                                                if( me.getHP() > 100){
                                                                        me.setHP(100);
                                                                }
                                                                R.Room_num = Cr.change_room;
                                                                R.Room_name = myRooms[i].getRname();
                                                                R.Room_des = myRooms[i].getRdes();
                                                                R.Room_des_len = myRooms[i].Room_des_len;
								R.setRnum(R.Room_num);
                                                                R.setRname(R.Room_name);
                                                                R.setRdes(R.Room_des);
                                                                writeRoom(R,client_fd);
								for(int j = 0; j < myRooms[i].conn.size(); j++){                 
                                                                        Co.Conn_Room_num = myRooms[myRooms[i].conn[j]].getRnum();
                                                                        Co.Conn_Room_name = myRooms[myRooms[i].conn[j]].getRname();
                                                                        Co.Conn_Room_des = myRooms[myRooms[i].conn[j]].getRdes();
                                                                        Co.Conn_Room_des_len = myRooms[myRooms[i].conn[j]].Room_des_len; 
                                                                        writeConnection(Co, client_fd);
                                                                }  
								 for(int j = client_fd; j >= 4; j--){
                                                                        for (int i = 0 ; i < Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(), j);
                                                                        }
                                                                }

                                                                if(size > Players.size()){

                                                                        for (int i = 0 ; i <  Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(),client_fd);
                                                                                }
                                                                }
                                                        }
							else if(Ch.Curr_Room_Num == 2 && (Cr.change_room == 1 || Cr.change_room == 5 || Cr.change_room == 10 || Cr.change_room == 11)){
                                                                Ch.Curr_Room_Num = Cr.change_room;
								me.setRoom(Ch.Curr_Room_Num);
								int Hp = me.getHP() + me.getReg();
                                                                me.setHP(Hp);
                                                                if( me.getHP() > 100){
                                                                        me.setHP(100);
                                                                }
                                                                R.Room_num = Cr.change_room;
                                                                R.Room_name = myRooms[i].getRname();
                                                                R.Room_des = myRooms[i].getRdes();
                                                                R.Room_des_len = myRooms[i].Room_des_len;
								R.setRnum(R.Room_num);
                                                                R.setRname(R.Room_name);
                                                                R.setRdes(R.Room_des);
                                                                writeRoom(R,client_fd);
								for(int j = 0; j < myRooms[i].conn.size(); j++){                 
                                                                        Co.Conn_Room_num = myRooms[myRooms[i].conn[j]].getRnum();
                                                                        Co.Conn_Room_name = myRooms[myRooms[i].conn[j]].getRname();
                                                                        Co.Conn_Room_des = myRooms[myRooms[i].conn[j]].getRdes();
                                                                        Co.Conn_Room_des_len = myRooms[myRooms[i].conn[j]].Room_des_len; 
                                                                        writeConnection(Co, client_fd);
								}
								for( int k = 0; k < Monsters.size(); k++){
                                                                        if (Monsters[k].getRoom() == Ch.Curr_Room_Num){
                                                                        mook.Char_Name = Monsters[k].getMName();
                                                                        mook.Flags = Monsters[k].getFlags();
                                                                        mook.Attack = Monsters[k].getAtt();
                                                                        mook.Defense = Monsters[k].getDef();
                                                                        mook.Regen = Monsters[k].getReg();
                                                                        mook.Health = Monsters[k].getHP();
                                                                        mook.Gold = Monsters[k].getGold();
                                                                        mook.Curr_Room_Num = Monsters[k].getRoom();
                                                                        mook.Char_des = Monsters[k].getDes();
                                                                        mook.Char_Des_len = Monsters[k].Char_Des_len;
                                                                        writeCharacter(mook,client_fd);
                                                                        }
                                                                }
								 for(int j = client_fd; j >= 4; j--){
                                                                        for (int i = 0 ; i < Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(), j);
                                                                        }
                                                                }

                                                                if(size > Players.size()){

                                                                        for (int i = 0 ; i <  Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(),client_fd);
                                                                                }
                                                                }
							}
							else if(Ch.Curr_Room_Num == 5 && (Cr.change_room == 6 || Cr.change_room == 2 )){
                                                                Ch.Curr_Room_Num = Cr.change_room;
								me.setRoom(Ch.Curr_Room_Num);
								int Hp = me.getHP() + me.getReg();
                                                                me.setHP(Hp);
                                                                if( me.getHP() > 100){
                                                                        me.setHP(100);
                                                                }
                                                                R.Room_num = Cr.change_room;
                                                                R.Room_name = myRooms[i].getRname();
                                                                R.Room_des = myRooms[i].getRdes();
                                                                R.Room_des_len = myRooms[i].Room_des_len;
								R.setRnum(R.Room_num);
                                                                R.setRname(R.Room_name);
                                                                R.setRdes(R.Room_des);
                                                                writeRoom(R,client_fd);
								for(int j = 0; j < myRooms[i].conn.size(); j++){                 
                                                                        Co.Conn_Room_num = myRooms[myRooms[i].conn[j]].getRnum();
                                                                        Co.Conn_Room_name = myRooms[myRooms[i].conn[j]].getRname();
                                                                        Co.Conn_Room_des = myRooms[myRooms[i].conn[j]].getRdes();
                                                                        Co.Conn_Room_des_len = myRooms[myRooms[i].conn[j]].Room_des_len; 
                                                                        writeConnection(Co, client_fd);
                                                                }
								for( int k = 0; k < Monsters.size(); k++){
                                                                        if (Monsters[k].getRoom() == Ch.Curr_Room_Num){
                                                                        mook.Char_Name = Monsters[k].getMName();
                                                                        mook.Flags = Monsters[k].getFlags();
                                                                        mook.Attack = Monsters[k].getAtt();
                                                                        mook.Defense = Monsters[k].getDef();
                                                                        mook.Regen = Monsters[k].getReg();
                                                                        mook.Health = Monsters[k].getHP();
                                                                        mook.Gold = Monsters[k].getGold();
                                                                        mook.Curr_Room_Num = Monsters[k].getRoom();
                                                                        mook.Char_des = Monsters[k].getDes();
                                                                        mook.Char_Des_len = Monsters[k].Char_Des_len;
                                                                        writeCharacter(mook,client_fd);
                                                                        }
                                                                }  
								for(int j = client_fd; j >= 4; j--){
                                                                        for (int i = 0 ; i < Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(), j);
                                                                        }
                                                                }

                                                                if(size > Players.size()){

                                                                        for (int i = 0 ; i <  Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(),client_fd);
                                                                                }
                                                                }
                                                        }
							else if(Ch.Curr_Room_Num == 11 && (Cr.change_room == 2)){
                                                                Ch.Curr_Room_Num = Cr.change_room;
								me.setRoom(Ch.Curr_Room_Num);
								int Hp = me.getHP() + me.getReg();
                                                                me.setHP(Hp);
                                                                if( me.getHP() > 100){
                                                                        me.setHP(100);
                                                                }
                                                                R.Room_num = Cr.change_room;
                                                                R.Room_name = myRooms[i].getRname();
                                                                R.Room_des = myRooms[i].getRdes();
                                                                R.Room_des_len = myRooms[i].Room_des_len;
								R.setRnum(R.Room_num);
                                                                R.setRname(R.Room_name);
                                                                R.setRdes(R.Room_des);
                                                                writeRoom(R,client_fd);
								for(int j = 0; j < myRooms[i].conn.size(); j++){                 
                                                                        Co.Conn_Room_num = myRooms[myRooms[i].conn[j]].getRnum();
                                                                        Co.Conn_Room_name = myRooms[myRooms[i].conn[j]].getRname();
                                                                        Co.Conn_Room_des = myRooms[myRooms[i].conn[j]].getRdes();
                                                                        Co.Conn_Room_des_len = myRooms[myRooms[i].conn[j]].Room_des_len; 
                                                                        writeConnection(Co, client_fd);
                                                                }  
								 for(int j = client_fd; j >= 4; j--){
                                                                        for (int i = 0 ; i < Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(), j);
                                                                        }
                                                                }

                                                                if(size > Players.size()){

                                                                        for (int i = 0 ; i <  Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(),client_fd);
                                                                                }
                                                                }
                                                        }
							else if(Ch.Curr_Room_Num == 10 && (Cr.change_room == 2)){
                                                                Ch.Curr_Room_Num = Cr.change_room;
								me.setRoom(Ch.Curr_Room_Num);
								int Hp = me.getHP() + me.getReg();
                                                                me.setHP(Hp);
                                                                if( me.getHP() > 100){
                                                                        me.setHP(100);
                                                                }
                                                                R.Room_num = Cr.change_room;
                                                                R.Room_name = myRooms[i].getRname();
                                                                R.Room_des = myRooms[i].getRdes();
                                                                R.Room_des_len = myRooms[i].Room_des_len;
								R.setRnum(R.Room_num);
                                                                R.setRname(R.Room_name);
                                                                R.setRdes(R.Room_des);
                                                                writeRoom(R,client_fd);
								for(int j = 0; j < myRooms[i].conn.size(); j++){                 
                                                                        Co.Conn_Room_num = myRooms[myRooms[i].conn[j]].getRnum();
                                                                        Co.Conn_Room_name = myRooms[myRooms[i].conn[j]].getRname();
                                                                        Co.Conn_Room_des = myRooms[myRooms[i].conn[j]].getRdes();
                                                                        Co.Conn_Room_des_len = myRooms[myRooms[i].conn[j]].Room_des_len; 
                                                                        writeConnection(Co, client_fd);
                                                                }  
								 for(int j = client_fd; j >= 4; j--){
                                                                        for (int i = 0 ; i < Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(), j);
                                                                        }
                                                                }

                                                                if(size > Players.size()){

                                                                        for (int i = 0 ; i <  Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(),client_fd);
                                                                                }
                                                                }
                                                        }
							else if(Ch.Curr_Room_Num == 6 && (Cr.change_room == 5 || Cr.change_room == 7)){
                                                                Ch.Curr_Room_Num = Cr.change_room;
								me.setRoom(Ch.Curr_Room_Num);
								int Hp = me.getHP() + me.getReg();
                                                                me.setHP(Hp);
                                                                if( me.getHP() > 100){
                                                                        me.setHP(100);
                                                                }
                                                                R.Room_num = Cr.change_room;
                                                                R.Room_name = myRooms[i].getRname();
                                                                R.Room_des = myRooms[i].getRdes();
                                                                R.Room_des_len = myRooms[i].Room_des_len;
								R.setRnum(R.Room_num);
                                                                R.setRname(R.Room_name);
                                                                R.setRdes(R.Room_des);
                                                                writeRoom(R,client_fd);
								for(int j = 0; j < myRooms[i].conn.size(); j++){                 
                                                                        Co.Conn_Room_num = myRooms[myRooms[i].conn[j]].getRnum();
                                                                        Co.Conn_Room_name = myRooms[myRooms[i].conn[j]].getRname();
                                                                        Co.Conn_Room_des = myRooms[myRooms[i].conn[j]].getRdes();
                                                                        Co.Conn_Room_des_len = myRooms[myRooms[i].conn[j]].Room_des_len; 
                                                                        writeConnection(Co, client_fd);
                                                                }
								for( int k = 0; k < Monsters.size(); k++){
                                                                        if (Monsters[k].getRoom() == Ch.Curr_Room_Num){
                                                                        mook.Char_Name = Monsters[k].getMName();
                                                                        mook.Flags = Monsters[k].getFlags();
                                                                        mook.Attack = Monsters[k].getAtt();
                                                                        mook.Defense = Monsters[k].getDef();
                                                                        mook.Regen = Monsters[k].getReg();
                                                                        mook.Health = Monsters[k].getHP();
                                                                        mook.Gold = Monsters[k].getGold();
                                                                        mook.Curr_Room_Num = Monsters[k].getRoom();
                                                                        mook.Char_des = Monsters[k].getDes();
                                                                        mook.Char_Des_len = Monsters[k].Char_Des_len;
                                                                        writeCharacter(mook,client_fd);
                                                                        }
                                                                }  
								for(int j = client_fd; j >= 4; j--){
                                                                        for (int i = 0 ; i < Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(), j);
                                                                        }
                                                                }

                                                                if(size > Players.size()){

                                                                        for (int i = 0 ; i <  Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(),client_fd);
                                                                                }
                                                                }
                                                        }
							else if(Ch.Curr_Room_Num == 7 && (Cr.change_room == 8 || Cr.change_room == 6 )){
                                                                Ch.Curr_Room_Num = Cr.change_room;
								me.setRoom(Ch.Curr_Room_Num);
								int Hp = me.getHP() + me.getReg();
                                                                me.setHP(Hp);
                                                                if( me.getHP() > 100){
                                                                        me.setHP(100);
                                                                }
                                                                R.Room_num = Cr.change_room;
                                                                R.Room_name = myRooms[i].getRname();
                                                                R.Room_des = myRooms[i].getRdes();
                                                                R.Room_des_len = myRooms[i].Room_des_len;
								R.setRnum(R.Room_num);
                                                                R.setRname(R.Room_name);
                                                                R.setRdes(R.Room_des);
                                                                writeRoom(R,client_fd);
								for(int j = 0; j < myRooms[i].conn.size(); j++){                 
                                                                        Co.Conn_Room_num = myRooms[myRooms[i].conn[j]].getRnum();
                                                                        Co.Conn_Room_name = myRooms[myRooms[i].conn[j]].getRname();
                                                                        Co.Conn_Room_des = myRooms[myRooms[i].conn[j]].getRdes();
                                                                        Co.Conn_Room_des_len = myRooms[myRooms[i].conn[j]].Room_des_len; 
                                                                        writeConnection(Co, client_fd);
                                                                }
								for( int k = 0; k < Monsters.size(); k++){
                                                                        if (Monsters[k].getRoom() == Ch.Curr_Room_Num){
                                                                        mook.Char_Name = Monsters[k].getMName();
                                                                        mook.Flags = Monsters[k].getFlags();
                                                                        mook.Attack = Monsters[k].getAtt();
                                                                        mook.Defense = Monsters[k].getDef();
                                                                        mook.Regen = Monsters[k].getReg();
                                                                        mook.Health = Monsters[k].getHP();
                                                                        mook.Gold = Monsters[k].getGold();
                                                                        mook.Curr_Room_Num = Monsters[k].getRoom();
                                                                        mook.Char_des = Monsters[k].getDes();
                                                                        mook.Char_Des_len = Monsters[k].Char_Des_len;
                                                                        writeCharacter(mook,client_fd);
                                                                        }
                                                                } 
								for(int j = client_fd; j >= 4; j--){
                                                                        for (int i = 0 ; i < Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(), j);
                                                                        }
                                                                }
								if(size > Players.size()){

                                                                        for (int i = 0 ; i <  Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(),client_fd);
                                                                                }
                                                                }
                                                        }
							else if(Ch.Curr_Room_Num == 8 && (Cr.change_room == 7)){
                                                                Ch.Curr_Room_Num = Cr.change_room;
								me.setRoom(Ch.Curr_Room_Num);
								int Hp = me.getHP() + me.getReg();
                                                                me.setHP(Hp);
                                                                if( me.getHP() > 100){
                                                                        me.setHP(100);
                                                                }
                                                                R.Room_num = Cr.change_room;
                                                                R.Room_name = myRooms[i].getRname();
                                                                R.Room_des = myRooms[i].getRdes();
                                                                R.Room_des_len = myRooms[i].Room_des_len;
								R.setRnum(R.Room_num);
                                                                R.setRname(R.Room_name);
                                                                R.setRdes(R.Room_des);
                                                                writeRoom(R,client_fd);
								for(int j = 0; j < myRooms[i].conn.size(); j++){                 
                                                                        Co.Conn_Room_num = myRooms[myRooms[i].conn[j]].getRnum();
                                                                        Co.Conn_Room_name = myRooms[myRooms[i].conn[j]].getRname();
                                                                        Co.Conn_Room_des = myRooms[myRooms[i].conn[j]].getRdes();
                                                                        Co.Conn_Room_des_len = myRooms[myRooms[i].conn[j]].Room_des_len; 
                                                                        writeConnection(Co, client_fd);
                                                                }
								for( int k = 0; k < Monsters.size(); k++){
                                                                        if (Monsters[k].getRoom() == Ch.Curr_Room_Num){
                                                                        mook.Char_Name = Monsters[k].getMName();
                                                                        mook.Flags = Monsters[k].getFlags();
                                                                        mook.Attack = Monsters[k].getAtt();
                                                                        mook.Defense = Monsters[k].getDef();
                                                                        mook.Regen = Monsters[k].getReg();
                                                                        mook.Health = Monsters[k].getHP();
                                                                        mook.Gold = Monsters[k].getGold();
                                                                        mook.Curr_Room_Num = Monsters[k].getRoom();
                                                                        mook.Char_des = Monsters[k].getDes();
                                                                        mook.Char_Des_len = Monsters[k].Char_Des_len;
                                                                        writeCharacter(mook,client_fd);
                                                                        }
                                                                }  
								for(int j = client_fd; j >= 4; j--){
                                                                        for (int i = 0 ; i < Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(), j);
                                                                        }
                                                                }

                                                                if(size > Players.size()){

                                                                        for (int i = 0 ; i <  Players.size(); i++){
                                                                                writeCharacter(Players[i]->getPlayer(),client_fd);
                                                                                }
                                                                }
                                                        }
							 else if(Ch.Curr_Room_Num == 4 && (Cr.change_room == 1)){
                                                                Ch.Curr_Room_Num = Cr.change_room;
								me.setRoom(Ch.Curr_Room_Num);
								int Hp = me.getHP() + me.getReg();
								me.setHP(Hp);
                                                                if( me.getHP() > 100){
                                                                       	me.setHP(100);
                                                                }
								//me.setHP(Ch.Health);
                                                                R.Room_num = Cr.change_room;
                                                                R.Room_name = myRooms[i].getRname();
                                                                R.Room_des = myRooms[i].getRdes();
                                                                R.Room_des_len = myRooms[i].Room_des_len;
								R.setRnum(R.Room_num);
                                                                R.setRname(R.Room_name);
                                                                R.setRdes(R.Room_des);
                                                                writeRoom(R,client_fd);
								for(int j = 0; j < myRooms[i].conn.size(); j++){                 
                                                                        Co.Conn_Room_num = myRooms[myRooms[i].conn[j]].getRnum();
                                                                        Co.Conn_Room_name = myRooms[myRooms[i].conn[j]].getRname();
                                                                        Co.Conn_Room_des = myRooms[myRooms[i].conn[j]].getRdes();
                                                                        Co.Conn_Room_des_len = myRooms[myRooms[i].conn[j]].Room_des_len; 
                                                                        writeConnection(Co, client_fd);
                                                                }  
								for(int j = client_fd; j >= 4; j--){
                                                                        for (int i = 0 ; i < Players.size(); i++){
                                                                        	writeCharacter(Players[i]->getPlayer(), j);
                                                                        }
                                                                }

                                                                if(size > Players.size()){

                                                                        for (int i = 0 ; i <  Players.size(); i++){
										if(me.getRoom() == Players[i]->getRoom()){
                                                                                        writeCharacter(Players[i]->getPlayer(), client_fd);
                                                                                }
                                                                        }
                                                                }

                                                        }

							else{
                                                       		E.Err_code = 1;
                                                        	writeError(E,client_fd);
								cout << "Did not move to room specified" << endl;
							}
						}
					}
				
                                }
				if(type == 1){
					cout << "READING A TYPE 1" << endl;
					read(client_fd,&M.Message_Len,2);
					char tbuff[M.Message_Len+1];
					char rbuff[33];
					bool send = false;
					read(client_fd,rbuff,32);
					rbuff[32] = 0;
					M.Rec_Name = rbuff;
					read(client_fd,rbuff,32);
					M.Sen_Name = rbuff;
					read(client_fd,tbuff,32);
					tbuff[M.Message_Len] = 0;
					M.The_Message = tbuff;
					A.Accepted   = type;
					cout << "Message: " << M.The_Message << endl;
					//for(int j = client_fd; j >= 4; j--){
						for( int i = 0; i < Players.size(); i++){
							if(M.Rec_Name == Players[i]->getName()){
								writeMessage(M,--client_fd);
								cout << "Socket: " << client_fd << endl;
								writeAccept(A,--client_fd);
								send = true;
							}

						}
					//}
					if(!send){
						cout << "No message sent..." << endl;
					}
					cout << "Message has been sent!" << endl;
                                }
				data_lock.unlock();
				if(type == 12){//
                                        me.RmChar(&me);
                                        close(client_fd);
                                        pthread_exit(&client_thread);
                                }
				if( Conn_check == 1){
					Conn_check = 0;
					close(client_fd);
					pthread_exit(&client_thread);
				}
				
                        }
                        close(client_fd);
                        client_fd = -1;
                        printf("Client %s disconnect\n", name);
                }
                ~client(){ 
                        client_thread.join();
                }
};




int main(int argc, char ** argv){
	Create_Rooms();
	Create_Monsters();
	for( int i = 0; i < myRooms.size();i++){
		cout << myRooms[i].getRname() << endl;
	}
	for( int i = 0; i < Monsters.size();i++){
                cout <<  Monsters[i].getMName() << endl;
        }
	struct sockaddr_in sad;
	if(argc > 1)
		sad.sin_port = htons(atoi(argv[1]));
	else {
		sad.sin_port = htons(5099);
		puts("Defaulting to port 5099");
	}
	sad.sin_addr.s_addr = INADDR_ANY;
	sad.sin_family = AF_INET;

	int skt = socket(AF_INET, SOCK_STREAM, 0);
	bind(skt, (struct sockaddr *)(&sad), sizeof(struct sockaddr_in));
	listen(skt, 5);
	int client_fd;
	struct sockaddr_in mad;
	socklen_t mad_size = sizeof(struct sockaddr_in);
	int type;
	for(;;){
		client_fd = accept(skt, (struct sockaddr *)&mad, &mad_size);
		cout << "connected!\n";
		data_lock.lock();
		clients.push_back(new client(client_fd));
		for(int i = 0; i < clients.size(); i++){
			if(clients[i]->client_fd == -1){
				auto tmp = clients[i];
				clients[i] = clients[clients.size()-1];
				clients.pop_back();
				delete tmp;
			}
		}
		data_lock.unlock();

	}
	return 0;
}
