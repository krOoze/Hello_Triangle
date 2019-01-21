up to [README.md](../README.md)

Quick Tutorial of basic synchronization of typical renderloop
-------------------------------------------------------------------

You will probably have two semaphors: e.g. `imageAcquired` and `renderingDone`.

(You would need more `imageAcquired` semaphores to prevent `vkAcquireNextImage`
to signal the same semaphore twice, but let's ignore this aspect for now.)

Let's cover it in an order in which things should actually get
**executed** (i.e. **not** necesserily in the order we called or recorded the
commands):

1. You provide `imageAcquired` semaphore to the `vkAcquireNextImageKHR` to be
signalled by it.

2. You provide `imageAcquired` semaphore to the command buffer submit to be
waited on. As a `pWaitDstStageMask` you provide the stage where your first write
access to the swapchain `VkImage` happens. Most often it will be
`VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` (but could be something different
based on your command buffer contents).

3. In the command buffer you change the layout of the swapchain image 
(Subpasses, Pipeline Barriers, or Events can do that) **from** the present one (
use `VK_IMAGE_LAYOUT_UNDEFINED` as an old layout, which means "sacrifice data"
&mdash; there's rarely ever need to read it after present ). For source stage
choose the exact same one as in step 2 and source access mask should be `0` (
because memory dependency is already part of the previous semaphore wait). For
efficient loop, destination stage should also be the same as in step 2 and the
new layout and destination access mask should be appropriate for whatever you
plan to do (likely `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL` and
`VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT`).

   In the case this is done with Render Pass (which should be the preferred
method when we need to draw something), the first use of the `VkImage` would be
the automatic layout transition followed by `loadOp` you provided during creation.
For color attachment load operation does occur in the
`VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` stage (so providing that value as
`dstStageMask` makes sure the layout transition does not clash with the load
operation). Access mask depends on the `loadOp` value chosen.

   The layout transition in render pass (and in Pipeline Barriers too) happens
between `srcStageMask` and `dstStageMask` stages. So we do not need to worry
about it too much, assuming our barrier\subpass dependency is otherwisely
correct.

4. You would write to your `VkImage`. Let's assume that is
done with a draw command like `vkCmdDraw` (which implies use of a Render Pass).
Now the `loadOp` guarantees that it happens before our first use of an attachment
in a render pass. So, no additional synchronization is needed here.

5. In the command buffer you change the layout of the swapchain image 
(again Subpasses, Pipeline Barriers, or Events can do that) **to** the present
one (`VK_IMAGE_LAYOUT_PRESENT_SRC_KHR`). As your source stage, old layout and
source access mask choose whatever your last use of the swapchain image was.
Destination stage should be `VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT` (i.e.
non-blocking, becose that is instead handled by folowing semaphore signal) and
the destination access mask should be `0`.

   In the case of this being a render pass, first a `storeOp` happens after the
last use of your image in a render pass. Again that is guaranteed and needs no
additional synchronization to order these two. The Store Operation for color
attachment happens in `VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` stage and
uses `VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT` access. The Store Operation is
followed by the automatic layout transition to `finalLayout`. So, to prevent
`storeOp` and this transition to clash, `srcStageMask` should be stage the Store
Op happens (`VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` as said above).

6. You provide `renderingDone` semaphore to the command buffer submit to be
signalled.

7. You provide `renderingDone` semaphore to the present command to be waited on.

8. The circle is now complete! :mask:

   From this it should be evident we are just declaring dependencies between
memory accesses (otherwisely Vulkan is allowed to mercilessly
overlap execution of commands). When there is any kind of memory read or write,
we must make sure any previous write to the same location has finished.
Usually, value used in `dstStageMask` subsequently appears in
following `srcStageMask` &mdash; so for a simple application it forms a nice
dependency chain or path or lifetime of image that is not so hard to reason
about:

Image Acquire  
=> Semaphore Signal  
=> Semaphore Unsignal in `VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT`  
=> `srcStage=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` stage of Subpass Dependency\Barrier  
=> Layout Transition From Present Layout  
=> `dstStage=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` stage of Subpass Dependency\Barrier  
=> Load Op (`VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` stage) => (Implicit Dependency)  
=> Draw (`VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` stage again\still)  
=> (Implicit Dependency) => Store Op (`VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` stage again\still)  
=> `srcStage` stage of Subpass Dependency\Barrier  
=> Layout Transition to Present Layout  
=> `dstStage=VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT` stage of Subpass Dependency\Barrier  
=> Semaphore Signal  
=> Semaphore Unsignal  
=> Presenting

From this diagram I can be sure the synchronization is correct:
- The pipeline stage of previous and next step matches
- or it is how the given primitive works (e.g. Semaphore wait does cover any
previously submitted semaphore signal)
- or I am given (relatively rare) specification guarantee that things implicitly
happen in specific order (e.g. the Load Op vs first subpass attachment use).