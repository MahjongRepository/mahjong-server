// �u�܂������v�p�ΐ푊��v���O�C��
//     �u�Ă���Ɓv
// �@�@�@�@�@�@�@�@�@�@�@�@�Δ� ����
#include <windows.h>
#include <stdio.h>
#include "../MIPIface.h"


class Tilt {
public :
	UINT InterfaceFunc(UINT message,UINT param1,UINT param2);

protected :
	int machi[34];
	char reach_flag[4];
	int te_cnt[34];
	int menzen;
	MJITehai tehai;
	int nakiok_flag;
	int sthai;
	int tehai_score;
	char anpai[34][4];
	int cha;
	int tenpai_flag;
	static int kyoku,kaze;

	int search(int obj,int start,int mask);
	void set_machi(void);
	void set_tehai(void);
	UINT sutehai_sub(int tsumohai);
	int eval_tehai_sub(int atama_flag);
	int eval_tehai(void);
	int eval_hai(int hai);
	int eval_sutehai(int hai);
	int calc_sutehai(void);
	int nakability(int hai,int chii_flag);
	UINT koe_req(int no,int hai);
	UINT on_start_kyoku(int k,int c);
	UINT on_end_kyoku(UINT reason,LONG* inc_sc);
	UINT on_action(int player,int taishou,UINT action);
	UINT on_start_game(void);
	UINT on_end_game(int rank,LONG score);
	UINT on_exchange(UINT state,UINT option);
};

int Tilt::kyoku,Tilt::kaze;

TCHAR player_name[] = TEXT("�Ă����");

UINT (WINAPI *MJSendMessage)(Tilt*,UINT,UINT,UINT);


// ��v�̒�����C�ӂ̔v�������A���̈ʒu��Ԃ�
int Tilt::search(int obj,int start,int mask)
{
	while(start<(int)tehai.tehai_max){
		if (!(mask&(1<<start)) && (int)tehai.tehai[start]==obj) break;
		start++;
	}
	return start<(int)tehai.tehai_max ? start : -1;
}

// ������v�𒲂ׁA�z��machi�ɓ����B
// �܂������Ă��邩�ǂ������ׂ�tenpai_flag���Z�b�g����B
void Tilt::set_machi(void)
{
	int i,j,cnt;
	(*MJSendMessage)(this,MJMI_GETMACHI,0,(UINT)machi);
	tenpai_flag = 0;
	for(i=0;i<34;i++){
		if (machi[i]) {
			cnt = 0;
			for(j=0;j<(int)tehai.tehai_max;j++) if ((int)tehai.tehai[j]==i) cnt++;
			if (cnt+(*MJSendMessage)(this,MJMI_GETVISIBLEHAIS,i,0)<4){
				tenpai_flag = 1;
				return;
			}
			tenpai_flag = -1;
		}
	}
}

// ��v�̕ϐ�tehai��te_cnt���Z�b�g����
void Tilt::set_tehai(void)
{
	int i;
	(*MJSendMessage)(this,MJMI_GETTEHAI,0,(UINT)&tehai);
	for(i=0;i<34;i++) te_cnt[i] = 0;
	for(i=0;i<(int)tehai.tehai_max;i++) te_cnt[tehai.tehai[i]]++;
}

// �̂Ĕv���̏���
// tsumohai : �������Ă����v
UINT Tilt::sutehai_sub(int tsumohai)
{
	int mc[34];
	UINT rchk=MJPIR_SUTEHAI;
	int i,hai,del_hai,hai_remain;

	// ���݂̎�v�̏�Ԃ��Z�b�g����
	set_tehai();
	
	// ���݂̑҂��v���擾����
	set_machi();

	// �c�������ꍇ�́u�c���v��
	if (tsumohai>=0 && tsumohai<34) if (machi[tsumohai]) return MJPIR_TSUMO;

	// ���[�`�������Ă���ꍇ�́u�c���؂�v
	if (reach_flag[0]) return MJPIR_SUTEHAI | 13;

	// ����v�ŗ�����ꍇ�͗���
	if ((*MJSendMessage)(this,MJMI_KKHAIABILITY,0,0)) return MJPIR_NAGASHI;
	
	// �����c�����Ă����v������Ȃ�A���̔v������
	if (tsumohai>=0 && tsumohai<34) te_cnt[tsumohai]++;
	
	// �̂Ă�v�����߂�
	hai = calc_sutehai();
	if (hai<(int)tehai.tehai_max) del_hai = tehai.tehai[hai]; else del_hai = tsumohai;
	
	// ��O�ŁA�e���p�����ꍇ�̓��[�`�������悤���Ȃ��H
	if (menzen){
		if (hai<(int)tehai.tehai_max){
			tehai.tehai[hai] = tsumohai;
		}
		hai_remain = (*MJSendMessage)(this,MJMI_GETHAIREMAIN,0,0);
		tenpai_flag = (*MJSendMessage)(this,MJMI_GETMACHI,(UINT)&tehai,(UINT)mc);
		for(i=0;i<34;i++){
			if (mc[i]){
				if (te_cnt[i]+(*MJSendMessage)(this,MJMI_GETVISIBLEHAIS,i,0)<4){
					if (hai_remain>60){ rchk = MJPIR_REACH; break;}
					if (!(*MJSendMessage)(this,MJMI_GETAGARITEN,(UINT)&tehai,i)){ rchk = MJPIR_REACH; break;}
				}
			}
		}
	}

	// �e��t���O�̃Z�b�g
	if (rchk==MJPIR_REACH) reach_flag[0] = 1;
	return hai|rchk;
}

// ��v�𕔕��I�ɕ]������
int Tilt::eval_tehai_sub(int atama_flag)
{
	int p=0,sc_max=0,sc,kazu,chk;
	char c;

	for(p=0;p<34;p++) if (te_cnt[p]) break;
	if (p==34) return 0;
	c = te_cnt[p];
	if (c>=3) {
		te_cnt[p]-=3;
		sc_max = eval_tehai_sub(atama_flag)+18;
		te_cnt[p]+=3;
	}
	if (c>=2) {
		te_cnt[p]-=2;
		sc = eval_tehai_sub(1)+6*(!anpai[p][0] || !atama_flag)+!atama_flag*12-(*MJSendMessage)(this,MJMI_GETVISIBLEHAIS,p,0);
		if (sc>sc_max) sc_max = sc;
		te_cnt[p]+=2;
	}
	if (p<27){
		kazu = p%9;
		if (kazu<7){
			if (te_cnt[p+2]){
				te_cnt[p]--; te_cnt[p+2]--;
				if (!anpai[p+1][0] && sthai!=p+1){
					sc = eval_tehai_sub(atama_flag)+8-(*MJSendMessage)(this,MJMI_GETVISIBLEHAIS,p+1,0);
					if (sc>sc_max) sc_max = sc;
					if (kazu<5) if (te_cnt[p+4]) if (!anpai[p+3][0] && sthai!=p+3){
						te_cnt[p+4]--;
						sc = eval_tehai_sub(atama_flag)+12;
						if (sc>sc_max) sc_max = sc;
						te_cnt[p+4]++;
					}
				}
				if (te_cnt[p+1]){
					te_cnt[p+1]--;
					sc = eval_tehai_sub(atama_flag)+18;
					if (sc>sc_max) sc_max = sc;
					te_cnt[p+1]++;
				}
				te_cnt[p]++; te_cnt[p+2]++;
			}
		}
		if (kazu<8){
			if (te_cnt[p+1]){
				chk = 0;
				if (kazu>0) if (anpai[p-1][0] || sthai==p-1) chk = 1;
				if (kazu<7) if (anpai[p+2][0] || sthai==p+2) chk = 1;
				if (!chk){
					te_cnt[p]--; te_cnt[p+1]--;
					sc = eval_tehai_sub(atama_flag)+8+(kazu>0 && kazu<7)*4;
					if (sc>sc_max) sc_max = sc;
					te_cnt[p]++; te_cnt[p+1]++;
				}
			}
		}
	}
	te_cnt[p]--;
	sc = eval_tehai_sub(atama_flag);
	if (sc>sc_max) sc_max = sc;
	te_cnt[p]++;
	return sc_max;
}

// ��v��]�����ĕ]���l��Ԃ�
int Tilt::eval_tehai(void)
{
	int ret = eval_tehai_sub(0);
	int i;
	for(i=0;i<34;i++){
		if (!te_cnt[i]) continue;
		ret += eval_hai(i)*te_cnt[i];
	}
	return ret;
}

// �v��]�����ĕ]���l��Ԃ�
int Tilt::eval_hai(int hai)
{
	int ret = 0,j,doras;
	UINT dora[6];

	if (hai<27) ret++; else {
		if ((*MJSendMessage)(this,MJMI_GETVISIBLEHAIS,hai,0)<2)
			if (hai>30 || hai==cha+27 || hai==kaze+27) ret++;
	}

	doras = (*MJSendMessage)(this,MJMI_GETDORA,(UINT)dora,0);
	for(j=0;j<doras;j++)
		if (hai==(int)dora[j]) ret++;
	return ret;
}

// �̂Ĕv��]�����ĕ]���l��Ԃ�
int Tilt::eval_sutehai(int hai)
{
	int i,ret = 0;

	for(i=1;i<4;i++){
		if (anpai[hai][i]) ret += 4;
		else if (hai<27){
			int kazu = hai%9,fl=1;
			if (kazu>2) if (!anpai[hai-3][i]) fl = 0;
			if (fl) if (kazu<6) if (!anpai[hai+3][i]) fl = 0;
			if (fl) ret += 3;
		}
		else if (!reach_flag[i]) ret += 2;
	}
	return ret;
}

// �̂Ă�v�����߂�
int Tilt::calc_sutehai(void)
{
	int i,ret;

	/*for(i=27;i<34;i++) if (te_cnt[i]==1) {
		ret = search(i,0,0);
		return ret>=0 ? ret : 13;
	}
	for(i=0;i<27;i++){
		if (!te_cnt[i] || te_cnt[i]>1) continue;
		kazu = i%9;
		if (kazu>0) if (te_cnt[i-1]) continue;
		if (kazu>1) if (te_cnt[i-2]) continue;
		if (kazu<8) if (te_cnt[i+1]) continue;
		if (kazu<7) if (te_cnt[i+2]) continue;
		ret = search(i,0,0);
		return ret>=0 ? ret : 13;
	}*/

	// �̂Ă�v���������Ă݂āA�����Ƃ��]���l�̍������̂��Ƃ�
	int sc_max = -1,sc,sh,scc,scc_max=-1;
	for(i=0;i<34;i++){
		if (!te_cnt[i]) continue;
		te_cnt[i]--;
		sthai = i;
		sc = eval_tehai();
		scc = sc + eval_sutehai(i);
		if (scc>scc_max){ scc_max = scc; sc_max = sc; sh = i;}
		te_cnt[i]++;
	}
	tehai_score = sc_max;
	ret = search(sh,0,0);

	return ret>=0 ? ret : 13;
}

// �����Ƃ��ł��邩�ǂ������ׂ�
// hai : �Ώۂ̔v
// chii_flag : �`�[�ɂ��Ă��`�F�b�N���邩�ǂ���
// return : �ȉ��̒l�̘_���a
//		 1 : �|���ł���
//		 2 : �J���ł���
//		 4 : �`�[�P�i���j�ł���
//		 8 : �`�[�Q�i�E�j�ł���
//		16 : �`�[�R�i���j�ł���
//		32 : �����ł���
int Tilt::nakability(int hai,int chii_flag)
{
	int x,ret=0,kazu;

	if (machi[hai]) ret |= 32;
	if (reach_flag[0]) return ret;
	if ((*MJSendMessage)(this,MJMI_GETHAIREMAIN,0,0)==0) return ret;
	if ((x=search(hai,0,0))>=0){
		if (x<(int)tehai.tehai_max-1){
			if ((int)tehai.tehai[x+1]==hai){
				ret |= 1;
				if (x<(int)tehai.tehai_max-2){
					if ((int)tehai.tehai[x+2]==hai) ret |= 2;
				}
			}
		}
	}
	if (chii_flag){
		if (hai<27){
			kazu = hai%9;
			if (kazu>1){
				if (te_cnt[hai-2]>0 && te_cnt[hai-1]>0) ret |= 8;
			}
			if (kazu<7){
				if (te_cnt[hai+2]>0 && te_cnt[hai+1]>0) ret |= 4;
			}
			if (kazu>0 && kazu<8){
				if (te_cnt[hai-1]>0 && te_cnt[hai+1]>0) ret |= 16;
			}
		}
	}
	return ret;
}

// ���Ƃ̎̂Ĕv�ɑ΂���A�N�V���������߂�
// no : ���ꂪ�̂Ă���
// hai : �����̂Ă���
// return : �A�N�V����
UINT Tilt::koe_req(int no,int hai)
{
	int chii_flag,sc;
	int naki_ok;

	set_tehai();
	set_machi();
	chii_flag = (no == 3);
	naki_ok = nakability(hai,chii_flag);
	if (!naki_ok) return 0;
	if (naki_ok&32){
		if ((*MJSendMessage)(this,MJMI_GETAGARITEN,0,hai)>0) {
			return MJPIR_RON;
		}
	}
	if (tenpai_flag==1) return 0;
	sthai = -1;
	if (naki_ok&1){
		if (hai>=27){
			if (te_cnt[hai]==2){
				if (hai>=31 || hai-27==cha || hai-27==kaze || nakiok_flag){
					return MJPIR_PON;
				}
			}
		} else {
			if (nakiok_flag){
				te_cnt[hai] -= 2;
				sc = eval_tehai();
				te_cnt[hai] += 2;
				if (sc+eval_hai(hai)*3+7>tehai_score) {
					return MJPIR_PON;
				}
			}
		}
	}
	if (!nakiok_flag) return 0;
	if (naki_ok&4){
		te_cnt[hai+1]--; te_cnt[hai+2]--;
		sc = eval_tehai()+eval_hai(hai)+eval_hai(hai+1)+eval_hai(hai+2);
		te_cnt[hai+1]++; te_cnt[hai+2]++;
		if (sc+7>tehai_score){
			return MJPIR_CHII1;
		}
	}
	if (naki_ok&8){
		te_cnt[hai-1]--; te_cnt[hai-2]--;
		sc = eval_tehai()+eval_hai(hai)+eval_hai(hai-1)+eval_hai(hai-2);
		te_cnt[hai-1]++; te_cnt[hai-2]++;
		if (sc+7>tehai_score){
			return MJPIR_CHII2;
		}
	}
	if (naki_ok&16){
		te_cnt[hai-1]--; te_cnt[hai+1]--;
		sc = eval_tehai()+eval_hai(hai)+eval_hai(hai+1)+eval_hai(hai-1);
		te_cnt[hai-1]++; te_cnt[hai+1]++;
		if (sc+7>tehai_score){
			return MJPIR_CHII3;
		}
	}
	return 0;
}

// �ǊJ�n���̏���
// k : ��
// c : ��
UINT Tilt::on_start_kyoku(int k,int c)
{
	int i,j;
	set_tehai();
	for(i=0;i<34;i++) {
		for (j=0;j<4;j++) anpai[i][j] = 0;
	}
	kyoku = k;
	kaze = kyoku/4;
	cha = c;
	menzen = 1;
	nakiok_flag = 0;
	sthai = -1;
	for(i=0;i<4;i++) reach_flag[i] = 0;
	tehai_score = eval_tehai();
	set_machi();
	return 0;
}

// �ǏI�����̏���
// reason : �I���������R
// inc_sc : �_���̕ω�
UINT Tilt::on_end_kyoku(UINT reason,LONG* inc_sc)
{
	if (*inc_sc>5000) (*MJSendMessage)(this,MJMI_FUKIDASHI,(UINT)TEXT("�悵�悵�c"),0);
	return 0;
}

// �A�N�V�����ɑ΂��鉞��������
UINT Tilt::on_action(int player,int taishou,UINT action)
{
	int hai = action&63;

	if (action & MJPIR_REACH) reach_flag[player] = 1;
	if (action & (MJPIR_SUTEHAI | MJPIR_REACH)){
		anpai[hai][player] = 1;
		for(int i=0;i<4;i++) if (reach_flag[i]) anpai[hai][i] = 1;
		if (player==0) return 0;
		return koe_req(player,hai);
	}
	if ((action & MJPIR_RON) && taishou==0){
		(*MJSendMessage)(this,MJMI_FUKIDASHI,(UINT)TEXT("�����c"),0);
	}
	if ((action & MJPIR_PON) && player==0){
		nakiok_flag = 1;
		menzen = 0;
	}
	if ((action & MJPIR_CHII1) && player==0){
		menzen = 0;
	}
	if ((action & MJPIR_CHII2) && player==0){
		menzen = 0;
	}
	if ((action & MJPIR_CHII3) && player==0){
		menzen = 0;
	}
	if ((action & MJPIR_MINKAN) && player==0){
		menzen = 0;
	}
	return 0;
}

// �����J�n���̏���
UINT Tilt::on_start_game(void)
{
	return 0;
}

// �����I�����̏���
// rank : ����
// score : �_��
UINT Tilt::on_end_game(int rank,LONG score)
{
	/*char str[40];
	sprintf(str,"%d�_�A%d�ʂ��c",score,rank+1);
	(*MJSendMessage)(this,MJMI_FUKIDASHI,(UINT)str,0);*/
	return 0;
}

// �r���Q�����̏���
// state : ���̂Ƃ��̏��
// option : ��ԂɊ֘A���đ�������
UINT Tilt::on_exchange(UINT state,UINT option)
{
	if (state==MJST_INKYOKU){
		int i,j,k;
		set_tehai();
		for(i=0;i<34;i++) {
			for (j=0;j<4;j++) anpai[i][j] = 0;
		}

		MJIKawahai kawa[30];
		for(i=0;i<4;i++){
			k = (*MJSendMessage)(this,MJMI_GETKAWAEX,MAKELPARAM(i,30),(UINT)kawa);
			reach_flag[i] = 0;
			for(j=0;j<k;j++){
				anpai[kawa[j].hai&63][j] = 1;
				if (kawa[j].state&MJKS_REACH) reach_flag[i] = 1;
			}
		}

		kyoku = LOWORD(option);
		kaze = kyoku/4;
		cha = HIWORD(option);
		menzen = tehai.minshun_max+tehai.minkan_max+tehai.minkou_max==0;
		nakiok_flag = !menzen;
		sthai = -1;
		tehai_score = eval_tehai();
		set_machi();
	}
	return 0;
}

// �C���X�^���X�p�̃C���^�[�t�F�[�X�֐�
UINT Tilt::InterfaceFunc(UINT message,UINT param1,UINT param2)
{
	switch(message){
	case MJPI_SUTEHAI :
		return sutehai_sub(LOWORD(param1));
	case MJPI_ONACTION :
		return on_action(LOWORD(param1),HIWORD(param1),param2);
	case MJPI_STARTKYOKU :
		return on_start_kyoku(LOWORD(param1),LOWORD(param2));
	case MJPI_ENDKYOKU :
		return on_end_kyoku(param1,(LONG*)param2);
	case MJPI_STARTGAME :
		return on_start_game();
	case MJPI_ENDGAME :
		return on_end_game(LOWORD(param1),(LONG)param2);
	case MJPI_ONEXCHANGE :
		return on_exchange(LOWORD(param1),param2);
	case MJPI_CREATEINSTANCE :
		return sizeof(Tilt);
	case MJPI_INITIALIZE :
		MJSendMessage = (UINT (WINAPI *)(Tilt*,UINT,UINT,UINT))param2;
		return 0;
	case MJPI_YOURNAME :
		return (UINT)player_name;
	case MJPI_DESTROY :
		return 0;
	case MJPI_ISEXCHANGEABLE :
		return 0; // �r���Q���ɑΉ�����B�Ή��������Ȃ��ꍇ��0�ȊO�ɂ���B
	}
	return MJR_NOTCARED;
}

// �C���^�[�t�F�[�X�֐�
UINT WINAPI MJPInterfaceFunc(Tilt* inst,UINT message,UINT param1,UINT param2)
{
	if (inst) return inst->InterfaceFunc(message,param1,param2);
	switch(message){
	case MJPI_CREATEINSTANCE :
		return sizeof(Tilt);
	case MJPI_INITIALIZE :
		MJSendMessage = (UINT (WINAPI *)(Tilt*,UINT,UINT,UINT))param2;
		return 0;
	case MJPI_YOURNAME :
		return (UINT)player_name;
	case MJPI_DESTROY :
		return 0;
	case MJPI_ISEXCHANGEABLE :
		return 0; // �r���Q���ɑΉ�����B�Ή��������Ȃ��ꍇ��0�ȊO�ɂ���B
	}
	return MJR_NOTCARED;
}
