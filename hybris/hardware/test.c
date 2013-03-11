/*
 * Copyright (C) 2012 Canonical Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Authored by: Michael Frey <michael.frey@canonical.com>
 */

#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "hardware.h"
#include "camera.h"

int main(int argc, char **argv)
{

  hw_module_t   *powerModule;
  hw_module_t   *sensorsModule;
  hw_module_t   *camera;
  camera_module_t   *cameraModule;
  camera_device_t   *mDevice = 0;

  char camera_device_name[10];
  snprintf(camera_device_name, sizeof(camera_device_name), "%d", 1);

  hw_get_module("gralloc", (const hw_module_t **)&powerModule);

  printf("Loaded module name %s\n", powerModule->name);
  printf("Loaded module version %d\n", powerModule->module_api_version);
  printf("Loaded module hal version %d\n", powerModule->hal_api_version);
  printf("Loaded module author %s\n", powerModule->author);
  printf("Loaded module id %s\n", powerModule->id);


  hw_get_module("camera", (const hw_module_t **)&cameraModule);
  struct camera_info info;
  int rc = cameraModule->get_camera_info(1, &info);
  printf("Camera facing = %d\n",info.facing);
  printf("Camera orientation = %d\n",info.orientation);

  printf("Loaded module number of cameras %d\n", cameraModule->get_number_of_cameras());
  camera = (hw_module_t *) &cameraModule->common;
  rc = camera->methods->open(camera, camera_device_name, (hw_device_t **)&mDevice);
  if (rc == 0) {
    printf("Preview enabled = %d\n", mDevice->ops->preview_enabled(mDevice));
    //mDevice->ops->start_preview(mDevice);
    mDevice->ops->take_picture(mDevice);
    printf("Took picture\n");
    rc = mDevice->common.close(&mDevice->common);
  } else 
    printf("Could not open camera\n");
  
  hw_get_module("sensors", (const hw_module_t **)&sensorsModule);
  printf("Loaded module name %s\n", sensorsModule->name);

}
