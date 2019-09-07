#include "pch.h"
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <cstdlib>
#include <algorithm>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
using namespace std;
using namespace CryptoPP;


const double PI = 3.1415;
const double e = 2.7182818;
const int NUM_OF_DATA = 1500;
const int maxn = NUM_OF_DATA + 5;
const int ALL_FEATURE = 15;
const int FEATURE = 14;
const int CLASSES = 2;
const int TestSetNumber = 100;
const int ClientNumber = 5;


struct BayesModel {
	double GuassianMean[FEATURE + 5];
	double GuassianDeviation[FEATURE + 5];
};

class Client {
private:
	BayesModel model;
	double GetProbability(double x, double mean, double deviation) {
		double p = 1 / sqrt(2 * PI*deviation*deviation)*pow(e, -((x - mean)*(x - mean)) / (2 * deviation*deviation));
		return p;
	}
	int GuassianNaivePredict(double *feature) {
		//Guassian Distributed Data
		//double GuassianMean[FEATURE + 5];
		//double GuassianDeviation[FEATURE + 5];
		for (int i = 0;i < ALL_FEATURE;i++) {
			double mean = -1, deviation = -1;
			double all = 0;
			for (int j = 0;j < NUM_OF_DATA;j++)
				all += Data[j][i];
			mean = all / NUM_OF_DATA;
			model.GuassianMean[i] = mean;
			double temp = 0;
			for (int j = 0;j < NUM_OF_DATA;j++)
				temp += pow(Data[j][i] - mean, 2);

			deviation = temp / NUM_OF_DATA;
			model.GuassianDeviation[i] = deviation;
		}


		int res = -1;
		double MaxProb = -1;

		for (int i = 0;i < CLASSES;i++) {
			double p = 1;
			for (int j = 0;j < FEATURE;j++) {
				p *= GetProbability(feature[j], model.GuassianMean[i], model.GuassianDeviation[i]);
				//cout << p << endl;
			}
			//cout <<endl<<endl<< "i is " << i << ": probability is " << p << endl;
			if (p > MaxProb) {
				res = i;
				MaxProb = p;
			}
		}
		//cout << "res is " << res << endl;
		return res;

	}
public:
	double Data[maxn][ALL_FEATURE];
	int HowManyData = 0;
	Client(){}
	
	void Train(){
		for (int i = 0;i < ALL_FEATURE;i++) {
			double mean = -1, deviation = -1;
			double all = 0;
			for (int j = 0;j < NUM_OF_DATA;j++)
				all += Data[j][i];
			mean = all / NUM_OF_DATA;
			model.GuassianMean[i] = mean;
			double temp = 0;
			for (int j = 0;j < NUM_OF_DATA;j++)
				temp += pow(Data[j][i] - mean, 2);

			deviation = temp / NUM_OF_DATA;
			model.GuassianDeviation[i] = deviation;
		}
	}
	int Predict(double *feature) {
		int res = -1;
		double MaxProb = -1;

		for (int i = 0;i < CLASSES;i++) {
			double p = 1;
			for (int j = 0;j < FEATURE;j++) {
				p *= GetProbability(feature[j], model.GuassianMean[i], model.GuassianDeviation[i]);
				//cout << p << endl;
			}
			//cout <<endl<<endl<< "i is " << i << ": probability is " << p << endl;
			if (p > MaxProb) {
				res = i;
				MaxProb = p;
			}
		}
		//cout << "res is " << res << endl;
		return res;
	}
	void ShowError() {
		int start = 0;
		int end = 100;
		int cnt = 0;

		for (int i = start;i < end;i++) {
			if (Predict(Data[i]) == Data[i][FEATURE])
				cnt++;
		}

		cout << "Accuracy：" << double(cnt) / abs(start - end);
	}
	void SaveAndEncrypt(){
		//save file
		ofstream f("BayesModel.txt");
		for (int i = 0;i < ALL_FEATURE;i++)
			f << model.GuassianMean[i] << " ";
		f<<endl;
		for (int i = 0;i < ALL_FEATURE;i++)
			f << model.GuassianDeviation[i] << " ";
		f << endl;

		//encrypt the file

	}
	void Sent(){}
};

class Server {
private:
	double Accuracies[ClientNumber+1];
	double Data[maxn][ALL_FEATURE];
	
	BayesModel model[ClientNumber+1];
	double GetProbability(double x, double mean, double deviation) {
		double p = 1 / sqrt(2 * PI*deviation*deviation)*pow(e, -((x - mean)*(x - mean)) / (2 * deviation*deviation));
		return p;
	}
public:
	Server(){
		
	}
	void Train(){}
	void Decrypt(){}
	void TestAll(){
		int correct[ClientNumber + 1] = { 0 };
		for (int i = 0;i < ClientNumber + 1;i++) {
			for (int j = 0;j < TestSetNumber;j++) {
				int num = rand() % maxn;
				if (Predict(model[i],Data[num]) == Data[num][FEATURE])
					correct[i]++;
			}
			Accuracies[i] = double(correct[i]) / TestSetNumber;
		}

	}
	int Predict(BayesModel m,double *feature) {
		int res = -1;
		double MaxProb = -1;

		for (int i = 0;i < CLASSES;i++) {
			double p = 1;
			for (int j = 0;j < FEATURE;j++) {
				p *= GetProbability(feature[j], m.GuassianMean[i], m.GuassianDeviation[i]);
			}
			if (p > MaxProb) {
				res = i;
				MaxProb = p;
			}
		}
		return res;
	}
};

class DataLoader{
	double Data[maxn][ALL_FEATURE];
public:
	double ToNum(string a) {
		double ans = 0;
		int pos = -1;
		for (int i = 0;i < a.length();i++) {
			if (a[i] == '.') {
				pos = i + 1;
				continue;
			}
			ans = ans * 10 + (a[i] - '0');
		}
		if (pos == -1) return ans;
		else return ans / pow(10, a.length() - pos);
	}
	DataLoader(string path) {
		ifstream FileIn;
		FileIn.open(path);
		if (!FileIn.is_open()) {
			cout << "cannot open the file." << endl;
		}


		string line;
		bool f = 0;
		int cnt = 0;
		while (getline(FileIn, line)) {
			if (cnt > NUM_OF_DATA) break;
			//cout << cnt << endl;
			vector<double> temp;

			int before = 0;
			for (int i = 0;i < line.length();i++) {
				if (line[i] == ',' || line[i] == '\n') {
					string sub = line.substr(before, i - before);

					temp.push_back(ToNum(sub));
					before = i + 1;
				}
			}
			temp.push_back(ToNum(line.substr(before, line.length() - before)));


			if (temp.size() < FEATURE) { cout << "data corrupted." << endl; }
			for (int i = 0;i < temp.size();i++)
				Data[cnt][i] = temp[i];
			cnt++;
		}
		cout << "data loading done. \ntotal number of data is: " << cnt << endl << endl;
	}
	void ShowData() {
		for (int i = 0;i < 10;i++) {
			for (int j = 0;j < ALL_FEATURE;j++)
				cout << Data[i][j] << " ";
			cout << endl;
		}
	}
	void AssignData(Client &c,int start,int end) {
		c.HowManyData = abs(end-start);
		for (int i = 0;i < c.HowManyData;i++)
			for (int j = 0;j < ALL_FEATURE;j++)
				c.Data[i][j] = Data[i][j];
	}
};

/*
byte key[CryptoPP::AES::DEFAULT_KEYLENGTH], iv[CryptoPP::AES::BLOCKSIZE];

void initKV(){
	memset(key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH);
	memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);
}

string encrypt(string plainText){
	string cipherText;

	CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);
	CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(cipherText));
	stfEncryptor.Put(reinterpret_cast<const unsigned char*>(plainText.c_str()), plainText.length() + 1);
	stfEncryptor.MessageEnd();

	string cipherTextHex;
	for (int i = 0; i < cipherText.size(); i++)
	{
		char ch[3] = { 0 };
		sprintf_s(ch, "%02x", static_cast<byte>(cipherText[i]));
		cipherTextHex += ch;
	}

	return cipherTextHex;
}



void writeCipher(string output){
	ofstream out("/tmp/cipher.data");
	out.write(output.c_str(), output.length());
	out.close();

	cout << "writeCipher finish " << endl << endl;
}
*/
int main() {
	srand((unsigned)time(NULL));
	
	/*string text = "mother fucker, I can finally work you out!";
	cout << "text : " << text << endl;

	initKV();
	string cipherHex = encrypt(text);
	cout << "cipher : " << cipherHex << endl;
	//writeCipher(cipherHex);
	*/
	
	DataLoader loader("E:\\NewPaper\\CplusplusSet\\adult_new.csv");
	//loader.ShowData();
	cout << "!!" << endl;
	getchar();
	Client client[ClientNumber];
	int s = 0, e = 1000;
	for (int i = 0;i < ClientNumber;i++) {
		
		loader.AssignData(client[i], s, e);
		e = s;
		s = s + 1000;
		client[i].Train();
		client[i].SaveAndEncrypt();
		client[i].ShowError();
	}
	

}