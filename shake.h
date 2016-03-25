/* 
 * SHAKE Copyright (c) 2016 Sebastien Serre <ssbx@sysmo.io>.
 *
 * This file is provided to you under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See
 * the License for the specific language governing permissions and limitations
 * under the License.
 */

/**
 * @file shake.h
 */
#ifndef SHAKE_H
#define SHAKE_H

/**
 * @brief Initialize shake
 * @param suggestedLatency PortAudio stream suggested latency
 * @return 0 if successfull
 */
int  shakeInit(float suggestedLatency);

/**
 * @brief Load wave file
 * @param fileName The wave file
 * @return The sound identifier
 */
int  shakeLoad(char* fileName);

/**
 * @brief Play a sound
 * @param soundId The sound identifier
 */
void shakePlay(int soundId);

/**
 * @brief Terminate shake
 */
void shakeTerminate();

#endif

