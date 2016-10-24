Quick Tutorial of basic synchronization of typical renderloop
-------------------------------------------------------------------

You will probably have two semaphors: e.g. `imageAcquired` and `renderingDone`.

Let's cover it in sort of an execution order:

1. You provide `imageAcquired` semaphore to the `vkAcquireNextImageKHR` to be signalled by it.

2. You provide `imageAcquired` semaphore to the command buffer submit to be waited on.
As a `pWaitDstStageMask` you provide the stage of your first write to the swapchain
`VkImage`. Most often it will be `VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT`
(but could be different based on your subpass implementation).

3. In the command buffer you change the layout of the swapchain image 
(Pipeline Barriers, Events or Subpasses do that) **from** the present one ( use 
`VK_IMAGE_LAYOUT_UNDEFINED` as an old layout, which means "sacrifice data" &mdash;
there's rarely ever need to read it after present ). For source stage choose
the exact same one as in 2. and source access mask should be `VK_ACCESS_MEMORY_READ_BIT`.
For efficient loop, destination stage should also be the same as in 2. and the
new layout and destination access mask should be appropriate for whatever you plan
to do (likely `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL` and `VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT`).

4. In the command buffer you change the layout of the swapchain image 
(Pipeline Barriers, Events or Subpasses do that) **to** the present one ( 
`VK_IMAGE_LAYOUT_PRESENT_SRC_KHR` ). For source stage, old layout and source
access mask choose whatever your last use of the swapchain image was.
Destination stage should be `VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT` (i.e. non-blocking)
and the destination access mask should be `VK_ACCESS_MEMORY_READ_BIT`.

5. You provide `renderingDone` semaphore to the command buffer submit to be signalled.

6. You provide `renderingDone` semaphore to the present command to be waited on.

7. The circle is now complete! :mask:
