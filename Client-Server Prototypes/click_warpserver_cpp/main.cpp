//
// Created by joel on 23.08.22.
//

#include "main.h"

using namespace clickhouse;

/// Initialize client connection.
Client client(ClientOptions().SetHost("10.5.1.31"));
//Client client(ClientOptions().SetHost("localhost"));



int clickserver_http_querycb(bool transferFormat, char *querystring){
    DLOG("Got request from client!\n");
    DLOG("Query: " << querystring);
    DLOG("Trying to open pipe to clickhouse server, query the server and print the output.");
    char commandstart[] = "clickhouse-client -h 10.5.1.31 -q ";
    char *command = (char *) malloc(strlen(commandstart)+ strlen(querystring)+2);
    sprintf(command, "%s'%s'", commandstart, querystring);
    DLOG("Command to be executed: " << command);
    FILE *pipe = popen(command, "r");
    char *buffer = (char *) malloc(1024);
    DLOG("----------------RESULT-------------------\n");
    while(nullptr != fgets(buffer, 1024, pipe)){
        DLOG(buffer);
    }
    pclose(pipe);
    free(buffer);
    free(command);
    return 0;
}

std::queue<unsigned char *> buffers;
std::queue<size_t> bufferSizes;
unsigned char *currentBuffer = nullptr;
Block *currentBlock = nullptr;
bool rowFormat = false;

void printTypeIDs(const Block& block){
    std::cout << "type ids of columns:" << std::endl;
    for(int i =0; i < block.GetColumnCount(); ++i){
        std::cout << " " << block[i]->GetType().GetName() << "-" << block[i]->GetType().GetCode() << std::endl;
    }
}

void printBlock(const Block& block){
    for (size_t i = 0; i < block.GetRowCount(); ++i) {
        std::cout << block[0]->As<ColumnInt32>()->At(i) << " "
                  << block[1]->As<ColumnString>()->At(i) << " "
                  << block[2]->As<ColumnString>()->At(i) << "\n";
    }
}

void writeRegionColumnBlock(const Block& block){}

void writeRegionRowBlock(const Block& block){
    if(block.GetRowCount() < 1) return;

    for(size_t i = 0; i < block.GetRowCount(); ++i){
        auto *nextBuf = (unsigned char *) calloc(DEFAULT_BUFFER_SIZE, 1);
        int bytesUsed = 0;
        int nextValLen = sizeof(block[0]->As<ColumnInt32>()->At(i));
        nextBuf[bytesUsed] = nextValLen;
        bytesUsed += sizeof(nextValLen);
        int32_t  nextint = (*block[0]->As<ColumnInt32>())[i];
        nextBuf[bytesUsed] = nextint;
        bytesUsed += nextValLen;

        nextValLen = block[1]->As<ColumnString>()->At(i).size();
        nextBuf[bytesUsed] = nextValLen;
        bytesUsed += sizeof(int);
        block[1]->As<ColumnString>()->At(i).copy((char *) (nextBuf+bytesUsed), nextValLen);
        bytesUsed += nextValLen;

        nextValLen = block[2]->As<ColumnString>()->At(i).size();
        nextBuf[bytesUsed] = nextValLen;
        bytesUsed += sizeof(int);
        block[2]->As<ColumnString>()->At(i).copy((char *) (nextBuf+bytesUsed), nextValLen);
        bytesUsed += nextValLen;

        buffers.push(nextBuf);
        bufferSizes.push(bytesUsed);
    }
}

void writeLineitemColumnBlock(const Block& block){}



void writeLineitemRowBlock(const Block& block){
    if(block.GetRowCount() < 1) return;

    auto *nextBuf = (unsigned char *) calloc(DEFAULT_BUFFER_SIZE, 1);
    int bytesUsed = 0;
    for(size_t i = 0; i < block.GetRowCount(); ++i){
        if((bytesUsed+14*4+100) > DEFAULT_BUFFER_SIZE){
            buffers.push(nextBuf);
            bufferSizes.push(bytesUsed);
            nextBuf = (unsigned char *) calloc(DEFAULT_BUFFER_SIZE, 1);
            bytesUsed = 0;
        }
        int32_t nextValLen = sizeof(block[0]->As<ColumnInt64>()->At(i));
        nextBuf[bytesUsed] = nextValLen;
        bytesUsed += sizeof(nextValLen);
        int64_t  nextint = (*block[0]->As<ColumnInt64>())[i];
        *((int64_t *)(nextBuf+bytesUsed)) = __bswap_constant_64(nextint);
        bytesUsed += nextValLen;

        nextValLen = sizeof(block[1]->As<ColumnInt32>()->At(i));
        nextBuf[bytesUsed] = nextValLen;
        bytesUsed += sizeof(nextValLen);
        *((int32_t *)(nextBuf+bytesUsed)) = __bswap_constant_32(block[1]->As<ColumnInt32>()->At(i));
        bytesUsed += nextValLen;

        nextValLen = sizeof(block[2]->As<ColumnInt32>()->At(i));
        nextBuf[bytesUsed] = nextValLen;
        bytesUsed += sizeof(nextValLen);
        *((int32_t *)(nextBuf+bytesUsed)) = __bswap_constant_32(block[2]->As<ColumnInt32>()->At(i));
        bytesUsed += nextValLen;

        nextValLen = sizeof(block[3]->As<ColumnInt32>()->At(i));
        nextBuf[bytesUsed] = nextValLen;
        bytesUsed += sizeof(nextValLen);
        *((int32_t *)(nextBuf+bytesUsed)) = __bswap_constant_32(block[3]->As<ColumnInt32>()->At(i));
        bytesUsed += nextValLen;

        nextValLen = block[4]->As<ColumnString>()->At(i).size();
        nextBuf[bytesUsed] = nextValLen;
        bytesUsed += sizeof(nextValLen);
        block[4]->As<ColumnString>()->At(i).copy((char *) (nextBuf+bytesUsed), nextValLen);
        bytesUsed += nextValLen;

        nextValLen = block[5]->As<ColumnString>()->At(i).size();
        nextBuf[bytesUsed] = nextValLen;
        bytesUsed += sizeof(nextValLen);
        block[5]->As<ColumnString>()->At(i).copy((char *) (nextBuf+bytesUsed), nextValLen);
        bytesUsed += nextValLen;

        nextValLen = block[6]->As<ColumnString>()->At(i).size();
        nextBuf[bytesUsed] = nextValLen;
        bytesUsed += sizeof(nextValLen);
        block[6]->As<ColumnString>()->At(i).copy((char *) (nextBuf+bytesUsed), nextValLen);
        bytesUsed += nextValLen;

        nextValLen = block[7]->As<ColumnString>()->At(i).size();
        nextBuf[bytesUsed] = nextValLen;
        bytesUsed += sizeof(nextValLen);
        block[7]->As<ColumnString>()->At(i).copy((char *) (nextBuf+bytesUsed), nextValLen);
        bytesUsed += nextValLen;

        nextValLen = block[8]->As<ColumnString>()->At(i).size();
        nextBuf[bytesUsed] = nextValLen;
        bytesUsed += sizeof(nextValLen);
        block[8]->As<ColumnString>()->At(i).copy((char *) (nextBuf+bytesUsed), nextValLen);
        bytesUsed += nextValLen;

    }
    buffers.push(nextBuf);
    bufferSizes.push(bytesUsed);
}

int clickserver_querycb(bool transferFormat, char *querystring){
    DLOG("Got request from client!");
    DLOG("Query: " << querystring);
    DLOG("Transferformat: " << (transferFormat ? "row" : "column"));
    rowFormat = transferFormat;
    if(rowFormat){
        //client.Select(querystring, writeRegionRowBlock);
        client.Select(querystring, writeLineitemRowBlock);
    }
    else {
        client.Select(querystring, writeRegionColumnBlock);
        //client.Select(querystring, writeLineitemColumnBlock);
    }

    return 0;
}

void clickserver_datacb(void **data, size_t *length){
    if(buffers.empty()) {
        DLOG("Sent all data...");
        *data = nullptr;
        *length = 0;
        free(currentBuffer);
        currentBuffer = nullptr;
        return;
    }
    if(currentBuffer != nullptr) free(currentBuffer);
    currentBuffer = buffers.front();
    buffers.pop();
    *data = currentBuffer;
    *length = bufferSizes.front();
    bufferSizes.pop();
}

int main() {
    DLOG("Clickserver, Spin On!");
    warpserver_initialize(nullptr, 13337, 0, clickserver_querycb, clickserver_datacb);
    warpserver_start();

    while(getchar() != '\n');

    DLOG("Stopping Clickserver.\n");
    warpserver_stop();
    return 0;
}
