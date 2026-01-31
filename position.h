#ifndef OTHELLO_POSITION_H
#define OTHELLO_POSITION_H
#include <cstdint>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <random>

inline std::mt19937 gen{std::random_device{}()};
inline std::uniform_int_distribution<uint64_t> int_dist(0,UINT64_MAX);
inline void disp_bb(const uint64_t &bb) {
    std::cout<<"\n";
    for (uint64_t i=0;i<64;i++) {
        if (i!=0&&i%8==0){std::cout<<"\n";}
        if (!(1ull<<i&bb)){std::cout<<"0 ";}
        else {std::cout<<"1 ";}
    }
    std::cout<<"\n\n";
}
inline void disp_bb(const uint8_t &bb) {
    std::cout<<"\n";
    for (uint64_t i=0;i<8;i++) {
        if (!(1ull<<i&bb)){std::cout<<"0 ";}
        else {std::cout<<"1 ";}
    }
    std::cout<<"\n\n";
}
namespace othello {
    static inline uint8_t lookup_table[256][256][8];
    static inline int coord_table[64][2]; //x,y
    static inline uint64_t zobrist_keys[129];
    static inline int sym_idx[64][7];
    inline uint8_t get_u8_h(const uint64_t &board,const int &row) {return (board>>row*8)&0xff;}
    inline uint64_t get_u64_h(const uint8_t &ray,const int &row) {return static_cast<uint64_t>(ray)<<row*8;}
    inline void generate_tables() {
        for (int p=0;p<256;p++) {
            for (int o=0;o<256;o++) {
                for (int s=0;s<8;s++) {
                    uint8_t flips=0;
                    uint8_t flips_temp=0;
                    uint8_t c=1<<s;
                    if (p&o||c&(p|o)) {
                        lookup_table[p][o][s]=0;
                        continue;
                    }
                    for (int i=0;i<6;i++) {
                        c=c<<1&0xfe&o;
                        flips_temp|=c;
                    }
                    if (flips_temp<<1&0xfe&p){flips|=flips_temp;}
                    flips_temp=0;
                    c=1<<s;
                    for (int i=0;i<6;i++) {
                        c=c>>1&0x7f&o;
                        flips_temp|=c;
                    }
                    if (flips_temp>>1&0x7f&p){flips|=flips_temp;}
                    lookup_table[p][o][s]=flips;
                }
            }
        }
        for (int i=0;i<64;i++) {
            coord_table[i][0]=i%8;
            coord_table[i][1]=i/8;
        }
        for (int i=0;i<129;i++) {
            zobrist_keys[i]=int_dist(gen);
        }
        for (int y=0;y<8;y++) {
            for (int x=0;x<8;x++) {
                int i=y*8+x;
                sym_idx[i][0]=y*8+(7-x);
                sym_idx[i][1]=(7-y)*8+x;
                sym_idx[i][2]=x*8+y;
                sym_idx[i][3]=(7-x)*8+(7-y);
                sym_idx[i][4]=x*8+(7-y);
                sym_idx[i][5]=(7-y)*8+(7-x);
                sym_idx[i][6]=(7-x)*8+y;
            }
        }
    }
    constexpr char row[8]={'A','B','C','D','E','F','G','H'};
    struct Move {
        bool side{};
        bool passing=false;
        uint64_t place=0ull;
        uint64_t flips=0ull;
        [[nodiscard]] std::string name() const {
            if (passing){return "Pass";}
            const int idx=__builtin_ctzll(place);
            return row[idx/8]+std::to_string(idx%8+1);
        }
        friend std::ostream &operator<<(std::ostream &stream, const Move &move) {
            stream<<move.name();
            return stream;
        }
        bool operator==(const Move &move) const {return move.flips==this->flips;}
    };
    class Position {
        int pass_counter=0;
        bool side_to_move=false;
        uint64_t board[2]={0x810000000,0x1008000000};
        uint64_t hash=0ull;
    public:
        [[nodiscard]] uint64_t psuedo_moves(const bool &side) const {
            uint64_t ps=board[!side];
            ps|=board[!side]<<1&0xfefefefefefefefe;
            ps|=board[!side]>>1&0x7f7f7f7f7f7f7f7f;
            ps|=board[!side]<<8&0xffffffffffffff00;
            ps|=board[!side]>>8&0xffffffffffffff;
            ps|=board[!side]>>7&0xfefefefefefefe;
            ps|=board[!side]>>9&0x7f7f7f7f7f7f7f;
            ps|=board[!side]<<7&0x7f7f7f7f7f7f7f00;
            ps|=board[!side]<<9&0xfefefefefefefe00;
            return ps&~occ();
        }
        //fen is in format: u64(black),u64(white),side_to_move
        explicit Position(const std::string &fen="startpos") { //set from fen
            if (lookup_table[0x81][0x76][3]!=0x76) {
                generate_tables();
            }
            hash=compute_hash();
            if (fen=="startpos"){return;}
            if (fen=="empty") {
                board[0]=0ull;
                board[1]=0ull;
                side_to_move=false;
                return;
            }
            std::vector<uint64_t> tokens;
            std::stringstream ss(fen);
            std::string token;
            while (std::getline(ss,token,',')) {
                tokens.push_back(std::stoull(token));
            }
            board[0]=tokens[0];
            board[1]=tokens[1];
            side_to_move=tokens[2];
        }
        [[nodiscard]] bool turn() const {return side_to_move;}
        [[nodiscard]] uint64_t occ() const {return board[0]|board[1];}
        [[nodiscard]] uint64_t empty() const {return ~occ();}
        [[nodiscard]] uint64_t side_occ(const bool &side) const {return board[side];}
        std::vector<Move> legal_moves() {
            std::vector<Move> move_list;
            uint64_t ps=psuedo_moves(turn());
            uint64_t flips=0ull;
            uint64_t flips_temp=0ull;
            while (ps) {
                const int lsb=__builtin_ctzll(ps);
                const uint64_t start_sqr=1ull<<lsb;
                ps^=start_sqr;
                flips=0ull;
                flips_temp=0ull;
                flips|=get_u64_h(lookup_table[get_u8_h(board[side_to_move],coord_table[lsb][1])][get_u8_h(board[!side_to_move],coord_table[lsb][1])][coord_table[lsb][0]],coord_table[lsb][1]);
                uint64_t candidate=start_sqr;
                for (int d=0;d<6;d++) {
                    candidate=candidate<<8&0xffffffffffffff00&board[!side_to_move];
                    if (!candidate){break;}
                    flips_temp|=candidate;
                }
                flips|=flips_temp&-!!(flips_temp<<8&0xffffffffffffff00&board[side_to_move]);
                flips_temp=0ull;
                candidate=start_sqr;
                for (int d=0;d<6;d++) {
                    candidate=candidate>>8&0xffffffffffffff&board[!side_to_move];
                    if (!candidate){break;}
                    flips_temp|=candidate;
                }
                flips|=flips_temp&-!!(flips_temp>>8&0xffffffffffffff&board[side_to_move]);
                flips_temp=0ull;
                candidate=start_sqr;
                for (int d=0;d<6;d++) {
                    candidate=candidate>>7&0xfefefefefefefe&board[!side_to_move];
                    if (!candidate){break;}
                    flips_temp|=candidate;
                }
                flips|=flips_temp&-!!(flips_temp>>7&0xfefefefefefefe&board[side_to_move]);
                flips_temp=0ull;
                candidate=start_sqr;
                for (int d=0;d<6;d++) {
                    candidate=candidate>>9&0x7f7f7f7f7f7f7f&board[!side_to_move];
                    if (!candidate){break;}
                    flips_temp|=candidate;
                }
                flips|=flips_temp&-!!(flips_temp>>9&0x7f7f7f7f7f7f7f&board[side_to_move]);
                flips_temp=0ull;
                candidate=start_sqr;
                for (int d=0;d<6;d++) {
                    candidate=candidate<<7&0x7f7f7f7f7f7f7f00&board[!side_to_move];
                    if (!candidate){break;}
                    flips_temp|=candidate;
                }
                flips|=flips_temp&-!!(flips_temp<<7&0x7f7f7f7f7f7f7f00&board[side_to_move]);
                flips_temp=0ull;
                candidate=start_sqr;
                for (int d=0;d<6;d++) {
                    candidate=candidate<<9&0xfefefefefefefe00&board[!side_to_move];
                    if (!candidate){break;}
                    flips_temp|=candidate;
                }
                flips|=flips_temp&-!!(flips_temp<<9& 0xfefefefefefefe00&board[side_to_move]);
                if (flips) {
                    move_list.emplace_back(side_to_move,false,start_sqr,flips);
                }
            }
            if (move_list.empty()){return {Move(side_to_move,true,0ull,0ull)};}
            return move_list;
        }
        void set_fen(const std::string &fen) {
            if (fen=="startpos") {
                board[0]=0x810000000;
                board[1]=0x1008000000;
                side_to_move=false;
                return;
            }
            if (fen=="empty") {
                board[0]=0ull;
                board[1]=0ull;
                side_to_move=false;
                return;
            }
            std::vector<uint64_t> tokens;
            std::stringstream ss(fen);
            std::string token;
            while (std::getline(ss,token,',')) {
                tokens.push_back(std::stoull(token));
            }
            board[0]=tokens[0];
            board[1]=tokens[1];
            side_to_move=tokens[2];
        }
        [[nodiscard]] std::string get_fen() const {return std::to_string(board[0])+","+std::to_string(board[1])+","+std::to_string(side_to_move);}
        [[nodiscard]] Position make_move(const Move &move) const {
            Position new_pos=*this;
            new_pos.board[side_to_move]^=move.place;
            new_pos.board[side_to_move]^=move.flips;
            new_pos.board[!side_to_move]^=move.flips;
            new_pos.pass_counter=(new_pos.pass_counter+1)*move.passing;
            new_pos.side_to_move=!side_to_move;
            //incremental hashing
            new_pos.hash^=zobrist_keys[__builtin_ctzll(move.place)+(side_to_move?64:0)];
            uint64_t flips=move.flips;
            while (flips) {
                const int lsb=__builtin_ctzll(flips);
                flips&=flips-1;
                new_pos.hash^=zobrist_keys[lsb];
                new_pos.hash^=zobrist_keys[lsb+64];
            }
            new_pos.hash^=zobrist_keys[128];
            return new_pos;
        }
        [[nodiscard]] bool is_terminal() const{
            //if two passes occur in sequence or you have no pieces or the opponenet has no pieces
            //or you have no pseudo moves and the opponent has no pseudo moves its terminal
            if (pass_counter>=2||board[0]==0||board[1]==0||(psuedo_moves(false)==0&&psuedo_moves(true)==0)){return true;}
            return false;
        }
        [[nodiscard]] uint64_t compute_hash() const {
            uint64_t h=0ull;
            for (int i=0;i<64;i++) {
                h^=zobrist_keys[i]&-!!(1ull<<i&board[0]);
                h^=zobrist_keys[i+64]&-!!(1ull<<i&board[1]);
            }
            return h^zobrist_keys[128]*side_to_move;
        }
        [[nodiscard]] uint64_t get_hash() const{return hash;}
        [[nodiscard]] uint64_t canonical_hash() const {
            uint64_t h[7]={0ull};
            for (int y=0;y<8;y++) {
                for (int x=0;x<8;x++) {
                    h[0]^=zobrist_keys[sym_idx[y*8+x][0]]&-!!(1ull<<sym_idx[y*8+x][0]&board[0]);
                    h[0]^=zobrist_keys[sym_idx[y*8+x][0]+64]&-!!(1ull<<sym_idx[y*8+x][0]&board[1]);
                    h[1]^=zobrist_keys[sym_idx[y*8+x][1]]&-!!(1ull<<sym_idx[y*8+x][1]&board[0]);
                    h[1]^=zobrist_keys[sym_idx[y*8+x][1]+64]&-!!(1ull<<sym_idx[y*8+x][1]&board[1]);
                    h[2]^=zobrist_keys[sym_idx[y*8+x][2]]&-!!(1ull<<sym_idx[y*8+x][2]&board[0]);
                    h[2]^=zobrist_keys[sym_idx[y*8+x][2]+64]&-!!(1ull<<sym_idx[y*8+x][2]&board[1]);
                    h[3]^=zobrist_keys[sym_idx[y*8+x][3]]&-!!(1ull<<sym_idx[y*8+x][3]&board[0]);
                    h[3]^=zobrist_keys[sym_idx[y*8+x][3]+64]&-!!(1ull<<sym_idx[y*8+x][3]&board[1]);
                    h[4]^=zobrist_keys[sym_idx[y*8+x][4]]&-!!(1ull<<sym_idx[y*8+x][4]&board[0]);
                    h[4]^=zobrist_keys[sym_idx[y*8+x][4]+64]&-!!(1ull<<sym_idx[y*8+x][4]&board[1]);
                    h[5]^=zobrist_keys[sym_idx[y*8+x][5]]&-!!(1ull<<sym_idx[y*8+x][5]&board[0]);
                    h[5]^=zobrist_keys[sym_idx[y*8+x][5]+64]&-!!(1ull<<sym_idx[y*8+x][5]&board[1]);
                    h[6]^=zobrist_keys[sym_idx[y*8+x][6]]&-!!(1ull<<sym_idx[y*8+x][6]&board[0]);
                    h[6]^=zobrist_keys[sym_idx[y*8+x][6]+64]&-!!(1ull<<sym_idx[y*8+x][6]&board[1]);
                }
            }
            for(int s=0;s<7;s++) h[s]^=zobrist_keys[128]*side_to_move;
            uint64_t min_hash=hash;
            for(int s=0;s<7;s++) min_hash=std::min(min_hash,h[s]);
            return min_hash;
        }
        friend std::ostream &operator<<(std::ostream &stream,const Position &pos) {
            stream<<"\n  1 2 3 4 5 6 7 8";
            for (uint64_t i=0;i<64;i++) {
                if (i%8==0) {
                    stream<<"\n"<<row[i/8]<<" "; }
                if (!(1ull<<i&pos.occ())){stream<<". ";}
                else if (1ull<<i&pos.board[0]){stream<<"X ";}
                else {stream<<"O ";}
            }
            stream<<"\nside to move: ";
            if (pos.side_to_move){stream<<"O\n\n";}
            else{stream<<"X\n\n";}
            return stream;
        }
    };
}





#endif //OTHELLO_POSITION_H